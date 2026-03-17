#include "TargetFrameCoreSubsystem.h"

#include "DynamicRHI.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h"
#include "HardwareInfo.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "RHI.h"
#include "RHIStats.h"
#include "Stats/Stats.h"
#include "TargetFrameCoreDeveloperSettings.h"
#include "TargetFrameCoreModule.h"

CSV_DEFINE_CATEGORY(TargetFrameCore, true);

namespace TargetFrameCorePrivate
{
    constexpr int64 BytesPerMegabyte = 1024ll * 1024ll;

    int32 BytesToMegabytes(int64 Bytes)
    {
        return Bytes > 0 ? static_cast<int32>(Bytes / BytesPerMegabyte) : 0;
    }

    int32 BytesToMegabytes(uint64 Bytes)
    {
        return Bytes > 0 ? static_cast<int32>(Bytes / BytesPerMegabyte) : 0;
    }

    FString VendorToString(ETargetFrameCoreGPUVendor Vendor)
    {
        switch (Vendor)
        {
        case ETargetFrameCoreGPUVendor::NVIDIA:
            return TEXT("NVIDIA");
        case ETargetFrameCoreGPUVendor::AMD:
            return TEXT("AMD");
        case ETargetFrameCoreGPUVendor::Intel:
            return TEXT("Intel");
        default:
            return TEXT("Unknown");
        }
    }

    FString TierToString(ETargetFrameCoreHardwareTier Tier)
    {
        switch (Tier)
        {
        case ETargetFrameCoreHardwareTier::Entry:
            return TEXT("Entry");
        case ETargetFrameCoreHardwareTier::Mainstream:
            return TEXT("Mainstream");
        case ETargetFrameCoreHardwareTier::Performance:
            return TEXT("Performance");
        default:
            return TEXT("Unknown");
        }
    }

    void AppendCapability(TArray<FString>& FeatureList, bool bSupported, const TCHAR* Label)
    {
        if (bSupported)
        {
            FeatureList.Add(Label);
        }
    }
}

UTargetFrameCoreSubsystem::UTargetFrameCoreSubsystem()
{
    Status.TargetFPS = 60;
    Status.OverallQualityLevel = 3;
    Status.ResolutionScale = 100.0f;
}

void UTargetFrameCoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_Initialize);

    Super::Initialize(Collection);

    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    Status.TargetFPS = Settings->DefaultTargetFPS;
    Status.bTelemetryEnabled = Settings->bEnableCsvTelemetry || Settings->MaxRecentEvents > 0;
    Status.bFireAndForgetShippingEnabled = IsFireAndForgetShippingEnabled();

    RefreshHardwareSnapshot();
    LoadStateFromGameUserSettings();
    if (Settings->bProbeProjectCapabilities || Status.bFireAndForgetShippingEnabled)
    {
        ProbeProjectCapabilities();
    }
    RefreshRuntimeStatusFromConsoleVariables();
    UpdateSetupSummary();

    bSubsystemInitialized = true;
    Status.bInitialized = true;

    AddRecentEvent(TEXT("TargetFrameCore subsystem initialized."));

    if (Settings->bAutoRunHardwareBenchmarkOnInitialize || Settings->bEnableRuntimeGovernor)
    {
        StartAutomaticSetup(false);
    }
}

void UTargetFrameCoreSubsystem::Deinitialize()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_Deinitialize);

    bSubsystemInitialized = false;
    Status.bInitialized = false;

    Super::Deinitialize();
}

void UTargetFrameCoreSubsystem::Tick(float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_Tick);

    if (!bSubsystemInitialized)
    {
        return;
    }

    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();

    ElapsedSeconds += DeltaTime;
    SecondsSinceLastEvaluation += DeltaTime;
    SecondsSinceLastRuntimeProbe += DeltaTime;

    const float InstantFPS = DeltaTime > UE_SMALL_NUMBER ? (1.0f / DeltaTime) : 0.0f;
    Status.SmoothedFPS = Status.SmoothedFPS <= 0.0f ? InstantFPS : FMath::Lerp(Status.SmoothedFPS, InstantFPS, 0.15f);
    Status.bTelemetryEnabled = Settings->bEnableCsvTelemetry || Settings->MaxRecentEvents > 0;

    RecordTelemetrySnapshot();

    if (Settings->bLogRuntimeProbe &&
        ElapsedSeconds >= Settings->WarmupSeconds &&
        SecondsSinceLastRuntimeProbe >= Settings->RuntimeProbeIntervalSeconds)
    {
        LogRuntimeProbe(TEXT("Periodic"));
        SecondsSinceLastRuntimeProbe = 0.0f;
    }

    if (!Settings->bEnableRuntimeGovernor)
    {
        return;
    }

    if (ElapsedSeconds < Settings->WarmupSeconds || SecondsSinceLastEvaluation < Settings->EvaluationIntervalSeconds)
    {
        return;
    }

    SecondsSinceLastEvaluation = 0.0f;
    EvaluateRuntimeBudget();
}

TStatId UTargetFrameCoreSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UTargetFrameCoreSubsystem, STATGROUP_Tickables);
}

bool UTargetFrameCoreSubsystem::IsTickable() const
{
    return bSubsystemInitialized && !IsTemplate();
}

bool UTargetFrameCoreSubsystem::IsTickableInEditor() const
{
    return false;
}

bool UTargetFrameCoreSubsystem::IsTickableWhenPaused() const
{
    return false;
}

void UTargetFrameCoreSubsystem::StartAutomaticSetup(bool bForceRebenchmark)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_StartAutomaticSetup);

    if (!bSubsystemInitialized)
    {
        return;
    }

    ElapsedSeconds = 0.0f;
    SecondsSinceLastEvaluation = 0.0f;
    SecondsSinceLastRuntimeProbe = 0.0f;
    Status.ConsecutiveLowSamples = 0;
    Status.ConsecutiveHighSamples = 0;
    Status.StableEvaluations = 0;
    Status.bPolicyLockEngaged = false;
    Status.bFireAndForgetShippingEnabled = IsFireAndForgetShippingEnabled();

    AddRecentEvent(
        bForceRebenchmark
            ? TEXT("Starting automatic setup with a forced hardware benchmark.")
            : TEXT("Starting automatic setup."));

    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    RefreshHardwareSnapshot();
    LoadStateFromGameUserSettings();
    if (Settings->bProbeProjectCapabilities || Status.bFireAndForgetShippingEnabled)
    {
        ProbeProjectCapabilities();
    }

    UGameUserSettings* GameUserSettings = ResolveUserSettings();

    const bool bHasBenchmarkScores =
        GameUserSettings != nullptr &&
        GameUserSettings->GetLastGPUBenchmarkResult() >= 0.0f &&
        GameUserSettings->GetLastCPUBenchmarkResult() >= 0.0f;

    const bool bShouldRunBenchmark =
        bForceRebenchmark ||
        (!bHasBenchmarkScores && Settings->bAutoRunHardwareBenchmarkOnInitialize);

    if (bShouldRunBenchmark && GameUserSettings != nullptr)
    {
        RunHardwareBenchmark();
    }
    else if (GameUserSettings != nullptr)
    {
        Status.LastCPUBenchmarkScore = GameUserSettings->GetLastCPUBenchmarkResult();
        Status.LastGPUBenchmarkScore = GameUserSettings->GetLastGPUBenchmarkResult();
        Status.bBenchmarkApplied = bHasBenchmarkScores;
    }
    else
    {
        AddRecentEvent(TEXT("GameUserSettings unavailable; TargetFrameCore is using project-agnostic fallback mode."));
    }

    ConfigureFireAndForgetShipping();
    ApplyCurrentPolicy();
}

void UTargetFrameCoreSubsystem::SetTargetFPS(int32 NewTargetFPS)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_SetTargetFPS);

    Status.TargetFPS = FMath::Clamp(NewTargetFPS, 15, 240);

    if (GetDefault<UTargetFrameCoreDeveloperSettings>()->bEnableDynamicResolution || ShouldForceDynamicResolution())
    {
        SetConsoleVariableFloat(TEXT("r.DynamicRes.FrameTimeBudget"), 1000.0f / FMath::Max(1, Status.TargetFPS));
    }

    UpdateSetupSummary();
    AddRecentEvent(FString::Printf(TEXT("Target FPS updated to %d."), Status.TargetFPS));
    RecordTelemetrySnapshot();
}

void UTargetFrameCoreSubsystem::ApplyCurrentPolicy()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_ApplyCurrentPolicy);

    UGameUserSettings* GameUserSettings = ResolveUserSettings();
    RefreshHardwareSnapshot();
    LoadStateFromGameUserSettings();

    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    Status.bFireAndForgetShippingEnabled = IsFireAndForgetShippingEnabled();
    if (Settings->bProbeProjectCapabilities || Status.bFireAndForgetShippingEnabled)
    {
        ProbeProjectCapabilities();
    }

    const bool bUseDynamicResolution =
        (!Status.ProjectCapabilities.bCapabilityScanComplete || Status.ProjectCapabilities.bSupportsDynamicResolution) &&
        (Settings->bEnableDynamicResolution || ShouldForceDynamicResolution());

    if (GameUserSettings != nullptr)
    {
        GameUserSettings->SetDynamicResolutionEnabled(bUseDynamicResolution);
    }

    if (!Status.ProjectCapabilities.bCapabilityScanComplete || Status.ProjectCapabilities.bSupportsDynamicResolution)
    {
        SetConsoleVariableInt(TEXT("r.DynamicRes.OperationMode"), bUseDynamicResolution ? 1 : 0);
    }

    if (bUseDynamicResolution)
    {
        const float EffectiveMinScale = FMath::Clamp(
            Settings->DynamicResolutionMinScreenPercentage,
            Settings->MinResolutionScale,
            Settings->DynamicResolutionMaxScreenPercentage);
        const float EffectiveMaxScale = FMath::Clamp(
            GetEffectiveMaxResolutionScale(),
            EffectiveMinScale,
            Settings->DynamicResolutionMaxScreenPercentage);

        SetConsoleVariableFloat(TEXT("r.DynamicRes.MinScreenPercentage"), EffectiveMinScale);
        SetConsoleVariableFloat(TEXT("r.DynamicRes.MaxScreenPercentage"), EffectiveMaxScale);
        SetConsoleVariableFloat(TEXT("r.DynamicRes.FrameTimeBudget"), 1000.0f / FMath::Max(1, Status.TargetFPS));
    }

    ApplyVendorAndMemoryPolicy();

    if (GameUserSettings != nullptr && (!Status.ProjectCapabilities.bCapabilityScanComplete || Status.ProjectCapabilities.bSupportsOverallQuality))
    {
        int32 CurrentOverallQuality = GameUserSettings->GetOverallScalabilityLevel();
        if (CurrentOverallQuality < 0)
        {
            CurrentOverallQuality = Settings->FallbackOverallQualityLevelWhenCustom;
        }

        const int32 EffectiveMaxQuality = GetEffectiveMaxOverallQualityLevel();
        if (CurrentOverallQuality > EffectiveMaxQuality)
        {
            GameUserSettings->SetOverallScalabilityLevel(EffectiveMaxQuality);
        }

        if (!Status.ProjectCapabilities.bCapabilityScanComplete || Status.ProjectCapabilities.bSupportsResolutionScale)
        {
            float CurrentNormalizedScale = 0.0f;
            float CurrentScale = 100.0f;
            float EngineMinScale = 0.0f;
            float EngineMaxScale = 100.0f;
            GameUserSettings->GetResolutionScaleInformationEx(CurrentNormalizedScale, CurrentScale, EngineMinScale, EngineMaxScale);

            const float EffectiveMaxResolutionScale = FMath::Clamp(GetEffectiveMaxResolutionScale(), EngineMinScale, EngineMaxScale);
            if (CurrentScale > EffectiveMaxResolutionScale)
            {
                GameUserSettings->SetResolutionScaleValueEx(EffectiveMaxResolutionScale);
            }
        }
    }
    ApplyUISeparationPolicy(bUseDynamicResolution);

    if (GameUserSettings != nullptr)
    {
        SaveCurrentSettings();
    }
    RefreshHardwareSnapshot();
    RefreshRuntimeStatusFromConsoleVariables();
    UpdateSetupSummary();

    RecordPolicyAction(TEXT("Applied automatic policy"), Status.SetupSummary);
    RecordTelemetrySnapshot();
}

FTargetFrameCoreStatus UTargetFrameCoreSubsystem::GetStatus() const
{
    return Status;
}

FTargetFrameCoreSetupSummary UTargetFrameCoreSubsystem::GetSetupSummary() const
{
    FTargetFrameCoreSetupSummary Summary;
    Summary.ProfileName = Status.RecommendedProfile;
    Summary.Summary = Status.SetupSummary;
    Summary.RecommendedTargetFPS = Status.TargetFPS;
    Summary.bLowVRAMMode = Status.bLowVRAMMode;
    Summary.bDynamicResolutionRecommended = Status.bDynamicResolutionManaged;
    Summary.bFireAndForgetShippingEnabled = Status.bFireAndForgetShippingEnabled;
    Summary.bPolicyLockEngaged = Status.bPolicyLockEngaged;
    Summary.SecondaryScreenPercentage = Status.SecondaryScreenPercentage;
    Summary.Notes = Status.SetupNotes;
    Summary.ProjectCapabilities = Status.ProjectCapabilities;
    return Summary;
}

void UTargetFrameCoreSubsystem::SetFireAndForgetShippingEnabled(bool bEnabled)
{
    bFireAndForgetShippingOverrideSet = true;
    bFireAndForgetShippingOverrideValue = bEnabled;
    Status.bFireAndForgetShippingEnabled = IsFireAndForgetShippingEnabled();
    Status.bPolicyLockEngaged = false;
    Status.StableEvaluations = 0;

    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    if (Settings->bProbeProjectCapabilities || Status.bFireAndForgetShippingEnabled)
    {
        ProbeProjectCapabilities();
    }

    UpdateSetupSummary();
    AddRecentEvent(
        Status.bFireAndForgetShippingEnabled
            ? TEXT("Fire-and-forget shipping capsule enabled for this session.")
            : TEXT("Fire-and-forget shipping capsule disabled for this session."));
    UE_LOG(
        LogTargetFrameCore,
        Log,
        TEXT("TargetFrameCore fire-and-forget shipping capsule %s."),
        Status.bFireAndForgetShippingEnabled ? TEXT("enabled") : TEXT("disabled"));
}

FTargetFrameCoreProjectCapabilities UTargetFrameCoreSubsystem::GetProjectCapabilities() const
{
    return Status.ProjectCapabilities;
}

void UTargetFrameCoreSubsystem::RefreshHardwareSnapshot()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_RefreshHardwareSnapshot);

    FTargetFrameCoreHardwareSnapshot Snapshot;
    Snapshot.PlatformName = ANSI_TO_TCHAR(FPlatformProperties::PlatformName());
    Snapshot.RHIName = FHardwareInfo::GetHardwareInfo(NAME_RHI);
    Snapshot.GPUBrand = FPlatformMisc::GetPrimaryGPUBrand();
    Snapshot.GPUVendor = DetectGPUVendor(Snapshot.GPUBrand);
    Snapshot.LogicalCoreCount = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
    Snapshot.SystemMemoryGB = static_cast<int32>(FPlatformMemory::GetConstants().TotalPhysical / (1024ull * 1024ull * 1024ull));
    Snapshot.bSupportsRayTracing = GRHISupportsRayTracing;
    Snapshot.bIsIntegratedGPU = GRHIDeviceIsIntegrated;

    if (GDynamicRHI != nullptr)
    {
        FTextureMemoryStats TextureMemoryStats;
        RHIGetTextureMemoryStats(TextureMemoryStats);

        Snapshot.DeviceWorkingMemoryMB = TargetFrameCorePrivate::BytesToMegabytes(TextureMemoryStats.GetTotalDeviceWorkingMemory());
        Snapshot.DedicatedVideoMemoryMB = TargetFrameCorePrivate::BytesToMegabytes(TextureMemoryStats.DedicatedVideoMemory);
        Snapshot.TotalGraphicsMemoryMB = TargetFrameCorePrivate::BytesToMegabytes(TextureMemoryStats.TotalGraphicsMemory);
        Snapshot.SharedSystemMemoryMB = TargetFrameCorePrivate::BytesToMegabytes(TextureMemoryStats.SharedSystemMemory);
        Snapshot.TexturePoolSizeMB = TargetFrameCorePrivate::BytesToMegabytes(TextureMemoryStats.TexturePoolSize);
        Snapshot.StreamingMemorySizeMB = TargetFrameCorePrivate::BytesToMegabytes(TextureMemoryStats.StreamingMemorySize);
        Snapshot.AvailableStreamingMemoryMB = TargetFrameCorePrivate::BytesToMegabytes(TextureMemoryStats.ComputeAvailableMemorySize());
    }

    Status.Hardware = Snapshot;
    Status.bLowVRAMMode = ShouldUseLowVRAMMode();
    Status.Hardware.RecommendedTier = DetermineHardwareTier(Status.Hardware);
}

void UTargetFrameCoreSubsystem::ProbeProjectCapabilities()
{
    FTargetFrameCoreProjectCapabilities Capabilities;
    Capabilities.bCapabilityScanComplete = true;

    const UGameUserSettings* GameUserSettings = ResolveUserSettings();
    Capabilities.bSupportsGameUserSettings = GameUserSettings != nullptr;
    Capabilities.bSupportsOverallQuality = Capabilities.bSupportsGameUserSettings;
    Capabilities.bSupportsResolutionScale = Capabilities.bSupportsGameUserSettings;
    Capabilities.bSupportsDynamicResolution =
        Capabilities.bSupportsGameUserSettings &&
        DoesConsoleVariableExist(TEXT("r.DynamicRes.OperationMode")) &&
        DoesConsoleVariableExist(TEXT("r.DynamicRes.FrameTimeBudget")) &&
        DoesConsoleVariableExist(TEXT("r.DynamicRes.MinScreenPercentage")) &&
        DoesConsoleVariableExist(TEXT("r.DynamicRes.MaxScreenPercentage"));
        DoesConsoleVariableExist(TEXT("r.Nanite.MaxPixelsPerEdge")) &&
        DoesConsoleVariableExist(TEXT("r.Nanite.PrimaryRaster.TimeBudgetMs")) &&
        DoesConsoleVariableExist(TEXT("r.Nanite.Tessellation"));
    Capabilities.bSupportsUpscalerSafeUI = DoesConsoleVariableExist(TEXT("r.SecondaryScreenPercentage.GameViewport"));
    Capabilities.bSupportsTexturePoolBudgeting =
        DoesConsoleVariableExist(TEXT("r.Streaming.PoolSize")) &&
        DoesConsoleVariableExist(TEXT("r.Streaming.LimitPoolSizeToVRAM"));

    TArray<FString> SupportedFeatures;
    TargetFrameCorePrivate::AppendCapability(SupportedFeatures, Capabilities.bSupportsOverallQuality, TEXT("quality tiers"));
    TargetFrameCorePrivate::AppendCapability(SupportedFeatures, Capabilities.bSupportsResolutionScale, TEXT("resolution scale"));
    TargetFrameCorePrivate::AppendCapability(SupportedFeatures, Capabilities.bSupportsDynamicResolution, TEXT("dynamic resolution"));
    TargetFrameCorePrivate::AppendCapability(SupportedFeatures, Capabilities.bSupportsUpscalerSafeUI, TEXT("UI separation"));
    TargetFrameCorePrivate::AppendCapability(SupportedFeatures, Capabilities.bSupportsTexturePoolBudgeting, TEXT("texture budgeting"));

    if (Capabilities.bSupportsGameUserSettings)
    {
        Capabilities.Notes.Add(TEXT("GameUserSettings is available for direct quality and resolution control."));
    }
    else
    {
        Capabilities.Notes.Add(TEXT("GameUserSettings is unavailable; TargetFrameCore will avoid quality-tier and resolution-scale writes."));
    }

    if (!Capabilities.bSupportsDynamicResolution)
    {
        Capabilities.Notes.Add(TEXT("Dynamic resolution controls are unavailable in this project runtime."));
    }

    {
        Capabilities.Notes.Add(TEXT("Nanite runtime budgeting controls are unavailable or disabled."));
    }

    if (!Capabilities.bSupportsUpscalerSafeUI)
    {
        Capabilities.Notes.Add(TEXT("Secondary screen percentage is unavailable; crisp UI separation is disabled."));
    }

    {
        Capabilities.Notes.Add(TEXT("Hardware ray tracing guard cvars are unavailable; TargetFrameCore will not force that toggle."));
    }

    if (!Capabilities.bSupportsTexturePoolBudgeting)
    {
        Capabilities.Notes.Add(TEXT("Texture pool budgeting cvars are unavailable; VRAM pool clamping is disabled."));
    }

    if (SupportedFeatures.IsEmpty())
    {
        Capabilities.Summary = TEXT("Project capsule is running in observation mode only.");
    }
    else if (Capabilities.Notes.Num() <= 1)
    {
        Capabilities.Summary = FString::Printf(
            TEXT("Project capsule detected full support for %s."),
            *FString::Join(SupportedFeatures, TEXT(", ")));
    }
    else
    {
        Capabilities.Summary = FString::Printf(
            TEXT("Project capsule will run in compatible mode with %s."),
            *FString::Join(SupportedFeatures, TEXT(", ")));
    }

    Status.ProjectCapabilities = MoveTemp(Capabilities);
}

void UTargetFrameCoreSubsystem::RunHardwareBenchmark()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_RunHardwareBenchmark);

    UGameUserSettings* GameUserSettings = ResolveUserSettings();
    if (GameUserSettings == nullptr)
    {
        return;
    }

    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();

    GameUserSettings->RunHardwareBenchmark(10, 1.0f, 1.0f);

    if (Settings->bAutoApplyHardwareBenchmarkResults)
    {
        GameUserSettings->ApplyHardwareBenchmarkResults();
    }

    Status.LastCPUBenchmarkScore = GameUserSettings->GetLastCPUBenchmarkResult();
    Status.LastGPUBenchmarkScore = GameUserSettings->GetLastGPUBenchmarkResult();
    Status.bBenchmarkApplied = true;

    AddRecentEvent(FString::Printf(
        TEXT("Hardware benchmark completed. CPU %.2f, GPU %.2f."),
        Status.LastCPUBenchmarkScore,
        Status.LastGPUBenchmarkScore));

    RefreshHardwareSnapshot();
    RefreshRuntimeStatusFromConsoleVariables();
    UpdateSetupSummary();

    if (Settings->bLogAdjustments)
    {
        UE_LOG(
            LogTargetFrameCore,
            Log,
            TEXT("TargetFrameCore benchmark completed. CPU=%.2f GPU=%.2f"),
            Status.LastCPUBenchmarkScore,
            Status.LastGPUBenchmarkScore);
    }
}

void UTargetFrameCoreSubsystem::LoadStateFromGameUserSettings()
{
    UGameUserSettings* GameUserSettings = ResolveUserSettings();
    if (GameUserSettings == nullptr)
    {
        return;
    }

    float CurrentNormalizedScale = 0.0f;
    float CurrentScale = 100.0f;
    float MinScale = 0.0f;
    float MaxScale = 100.0f;
    GameUserSettings->GetResolutionScaleInformationEx(CurrentNormalizedScale, CurrentScale, MinScale, MaxScale);

    Status.ResolutionScale = CurrentScale;
    Status.OverallQualityLevel = GameUserSettings->GetOverallScalabilityLevel();
    Status.LastCPUBenchmarkScore = GameUserSettings->GetLastCPUBenchmarkResult();
    Status.LastGPUBenchmarkScore = GameUserSettings->GetLastGPUBenchmarkResult();
    Status.bBenchmarkApplied = Status.LastCPUBenchmarkScore >= 0.0f && Status.LastGPUBenchmarkScore >= 0.0f;
    Status.bDynamicResolutionManaged = GameUserSettings->IsDynamicResolutionEnabled();
}

void UTargetFrameCoreSubsystem::SaveCurrentSettings()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_SaveCurrentSettings);

    UGameUserSettings* GameUserSettings = ResolveUserSettings();
    if (GameUserSettings == nullptr)
    {
        return;
    }

    GameUserSettings->ApplySettings(false);
    GameUserSettings->SaveSettings();
    LoadStateFromGameUserSettings();
    RefreshRuntimeStatusFromConsoleVariables();
    UpdateSetupSummary();
}

void UTargetFrameCoreSubsystem::EvaluateRuntimeBudget()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_EvaluateRuntimeBudget);

    RefreshHardwareSnapshot();
    RefreshRuntimeStatusFromConsoleVariables();
    UpdateSetupSummary();

    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    const float LowThreshold = static_cast<float>(Status.TargetFPS) - Settings->AllowedDeficitFPS;
    const float HighThreshold = static_cast<float>(Status.TargetFPS) + Settings->RecoveryHeadroomFPS;

    if (Status.bFireAndForgetShippingEnabled && Settings->bLockPolicyAfterStabilization && Status.bPolicyLockEngaged)
    {
        const float UnlockThreshold = static_cast<float>(Status.TargetFPS) - Settings->SevereDeficitFPSBeforeUnlock;
        if (Status.SmoothedFPS >= UnlockThreshold)
        {
            return;
        }

        Status.bPolicyLockEngaged = false;
        Status.StableEvaluations = 0;
        AddRecentEvent(TEXT("Fire-and-forget lock released after a severe frame deficit."));
        UE_LOG(LogTargetFrameCore, Log, TEXT("TargetFrameCore fire-and-forget lock released after severe FPS deficit."));
    }

    if (Status.SmoothedFPS < LowThreshold)
    {
        if (Status.bFireAndForgetShippingEnabled && Settings->bLockPolicyAfterStabilization && !CanReduceCostFurther())
        {
            ++Status.StableEvaluations;
            Status.ConsecutiveLowSamples = 0;
            Status.ConsecutiveHighSamples = 0;

            if (!Status.bPolicyLockEngaged && Status.StableEvaluations >= Settings->StableEvaluationsBeforeLock)
            {
                Status.bPolicyLockEngaged = true;
                AddRecentEvent(TEXT("Fire-and-forget lock engaged because no cheaper fallback remains."));
                UE_LOG(LogTargetFrameCore, Log, TEXT("TargetFrameCore fire-and-forget lock engaged at cost floor."));
            }

            return;
        }

        Status.StableEvaluations = 0;
        ++Status.ConsecutiveLowSamples;
        Status.ConsecutiveHighSamples = 0;

        if (Status.ConsecutiveLowSamples >= Settings->ConsecutiveLowSamplesRequired)
        {
            StepQualityDown();
            Status.ConsecutiveLowSamples = 0;
        }

        return;
    }

    if (Status.SmoothedFPS > HighThreshold)
    {
        if (Status.bFireAndForgetShippingEnabled && Settings->bLockPolicyAfterStabilization && !CanIncreaseQualityFurther())
        {
            ++Status.StableEvaluations;
            Status.ConsecutiveLowSamples = 0;
            Status.ConsecutiveHighSamples = 0;

            if (!Status.bPolicyLockEngaged && Status.StableEvaluations >= Settings->StableEvaluationsBeforeLock)
            {
                Status.bPolicyLockEngaged = true;
                AddRecentEvent(TEXT("Fire-and-forget lock engaged at the quality ceiling."));
                UE_LOG(LogTargetFrameCore, Log, TEXT("TargetFrameCore fire-and-forget lock engaged at quality ceiling."));
            }

            return;
        }

        Status.StableEvaluations = 0;
        ++Status.ConsecutiveHighSamples;
        Status.ConsecutiveLowSamples = 0;

        if (Status.ConsecutiveHighSamples >= Settings->ConsecutiveHighSamplesRequired)
        {
            StepQualityUp();
            Status.ConsecutiveHighSamples = 0;
        }

        return;
    }

    Status.ConsecutiveLowSamples = 0;
    Status.ConsecutiveHighSamples = 0;

    if (Status.bFireAndForgetShippingEnabled && Settings->bLockPolicyAfterStabilization)
    {
        ++Status.StableEvaluations;
        if (!Status.bPolicyLockEngaged && Status.StableEvaluations >= Settings->StableEvaluationsBeforeLock)
        {
            Status.bPolicyLockEngaged = true;
            AddRecentEvent(TEXT("Fire-and-forget lock engaged after a stable settling window."));
            UE_LOG(
                LogTargetFrameCore,
                Log,
                TEXT("TargetFrameCore fire-and-forget lock engaged after %d stable evaluations."),
                Status.StableEvaluations);
        }
    }
}

void UTargetFrameCoreSubsystem::StepQualityDown()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_StepQualityDown);

    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    const FString Reason = FString::Printf(
        TEXT("Smoothed FPS %.1f dropped below %.1f."),
        Status.SmoothedFPS,
        static_cast<float>(Status.TargetFPS) - Settings->AllowedDeficitFPS);

    if (Settings->bAdjustResolutionScale && TryAdjustResolutionScale(-Settings->ResolutionScaleStep))
    {
        RecordPolicyAction(TEXT("Lowered resolution scale"), Reason);
        RecordTelemetrySnapshot();
        return;
    }
if (Settings->bAdjustOverallQuality && TryAdjustOverallQuality(-1))
    {
        RecordPolicyAction(TEXT("Reduced overall scalability level"), Reason);
        RecordTelemetrySnapshot();
        return;
    }

    AddRecentEvent(FString::Printf(TEXT("No lower-cost runtime fallback remained at %.1f FPS."), Status.SmoothedFPS));
}

void UTargetFrameCoreSubsystem::StepQualityUp()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_StepQualityUp);

    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    const FString Reason = FString::Printf(
        TEXT("Smoothed FPS %.1f exceeded %.1f."),
        Status.SmoothedFPS,
        static_cast<float>(Status.TargetFPS) + Settings->RecoveryHeadroomFPS);

    if (Settings->bAdjustOverallQuality && TryAdjustOverallQuality(1))
    {
        RecordPolicyAction(TEXT("Raised overall scalability level"), Reason);
        RecordTelemetrySnapshot();
        return;
    }
if (Settings->bAdjustResolutionScale && TryAdjustResolutionScale(Settings->ResolutionScaleStep))
    {
        RecordPolicyAction(TEXT("Restored resolution scale"), Reason);
        RecordTelemetrySnapshot();
        return;
    }

    AddRecentEvent(FString::Printf(TEXT("No higher-quality step was available at %.1f FPS."), Status.SmoothedFPS));
}

void UTargetFrameCoreSubsystem::ApplyVendorAndMemoryPolicy()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_ApplyVendorAndMemoryPolicy);

    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    Status.bVendorPolicyApplied = Settings->bEnableVendorAndVRAMPolicy;

    if (!Settings->bEnableVendorAndVRAMPolicy)
    {
        return;
    }

    if (Status.ProjectCapabilities.bCapabilityScanComplete && !Status.ProjectCapabilities.bSupportsTexturePoolBudgeting)
    {
        return;
    }

    if (Settings->bLimitTexturePoolToVRAM)
    {
        SetConsoleVariableInt(TEXT("r.Streaming.LimitPoolSizeToVRAM"), 1);

        const int32 GraphicsMemoryMB =
            Status.Hardware.DeviceWorkingMemoryMB > 0 ? Status.Hardware.DeviceWorkingMemoryMB : Status.Hardware.TotalGraphicsMemoryMB;
        if (GraphicsMemoryMB > 0)
        {
            const int32 MaxReasonablePoolMB = FMath::Max(256, GraphicsMemoryMB - 256);
            const int32 PreferredPoolMB = FMath::RoundToInt(static_cast<float>(GraphicsMemoryMB) * Settings->TexturePoolBudgetFraction);
            const int32 MinPoolMB = FMath::Min(Settings->MinimumTexturePoolSizeMB, MaxReasonablePoolMB);
            const int32 PoolBudgetMB = FMath::Clamp(PreferredPoolMB, MinPoolMB, MaxReasonablePoolMB);

            SetConsoleVariableInt(TEXT("r.Streaming.PoolSize"), PoolBudgetMB);
            Status.TexturePoolBudgetMB = PoolBudgetMB;
        }
    }
    else
    {
        SetConsoleVariableInt(TEXT("r.Streaming.LimitPoolSizeToVRAM"), 0);
    }
}

void UTargetFrameCoreSubsystem::ApplyUISeparationPolicy(bool bDynamicResolutionEnabled)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TargetFrameCore_ApplyUISeparationPolicy);

    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    if (Status.ProjectCapabilities.bCapabilityScanComplete && !Status.ProjectCapabilities.bSupportsUpscalerSafeUI)
    {
        Status.SecondaryScreenPercentage = Settings->SecondaryScreenPercentageWhenNative;
        Status.bUpscalerSafeUIEnabled = false;
        return;
    }

    if (!Settings->bEnableUpscalerSafeUI)
    {
        SetConsoleVariableFloat(TEXT("r.SecondaryScreenPercentage.GameViewport"), Settings->SecondaryScreenPercentageWhenNative);
        RefreshRuntimeStatusFromConsoleVariables();
        return;
    }

    const bool bUpscalingActive = bDynamicResolutionEnabled || Status.ResolutionScale < 99.5f;
    const float SecondaryScreenPercentage =
        (!Settings->bOnlySeparateUIWhenUpscaling || bUpscalingActive)
            ? Settings->SecondaryScreenPercentageForUI
            : Settings->SecondaryScreenPercentageWhenNative;

    SetConsoleVariableFloat(TEXT("r.SecondaryScreenPercentage.GameViewport"), SecondaryScreenPercentage);
    RefreshRuntimeStatusFromConsoleVariables();
}

bool UTargetFrameCoreSubsystem::TryAdjustResolutionScale(float DeltaScale)
{
    if (Status.ProjectCapabilities.bCapabilityScanComplete && !Status.ProjectCapabilities.bSupportsResolutionScale)
    {
        return false;
    }

    if (Status.bDynamicResolutionManaged)
    {
        return false;
    }

    UGameUserSettings* GameUserSettings = ResolveUserSettings();
    if (GameUserSettings == nullptr)
    {
        return false;
    }

    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();

    float CurrentNormalizedScale = 0.0f;
    float CurrentScale = 100.0f;
    float EngineMinScale = 0.0f;
    float EngineMaxScale = 100.0f;
    GameUserSettings->GetResolutionScaleInformationEx(CurrentNormalizedScale, CurrentScale, EngineMinScale, EngineMaxScale);

    const float PolicyMinScale = FMath::Max(Settings->MinResolutionScale, EngineMinScale);
    const float PolicyMaxScale = FMath::Min(GetEffectiveMaxResolutionScale(), EngineMaxScale);
    const float NewScale = FMath::Clamp(CurrentScale + DeltaScale, PolicyMinScale, PolicyMaxScale);

    if (FMath::IsNearlyEqual(NewScale, CurrentScale, KINDA_SMALL_NUMBER))
    {
        return false;
    }

    GameUserSettings->SetResolutionScaleValueEx(NewScale);
    SaveCurrentSettings();

    return true;
}

bool UTargetFrameCoreSubsystem::TryAdjustOverallQuality(int32 DeltaLevel)
{
    if (Status.ProjectCapabilities.bCapabilityScanComplete && !Status.ProjectCapabilities.bSupportsOverallQuality)
    {
        return false;
    }

    UGameUserSettings* GameUserSettings = ResolveUserSettings();
    if (GameUserSettings == nullptr)
    {
        return false;
    }

    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();

    int32 CurrentLevel = GameUserSettings->GetOverallScalabilityLevel();
    if (CurrentLevel < 0)
    {
        CurrentLevel = Settings->FallbackOverallQualityLevelWhenCustom;
    }

    const int32 NewLevel = FMath::Clamp(CurrentLevel + DeltaLevel, Settings->MinOverallQualityLevel, GetEffectiveMaxOverallQualityLevel());
    if (NewLevel == CurrentLevel)
    {
        return false;
    }

    GameUserSettings->SetOverallScalabilityLevel(NewLevel);
    SaveCurrentSettings();

    return true;
}

void UTargetFrameCoreSubsystem::RefreshRuntimeStatusFromConsoleVariables()
{
    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();

    Status.SecondaryScreenPercentage = (!Status.ProjectCapabilities.bCapabilityScanComplete || Status.ProjectCapabilities.bSupportsUpscalerSafeUI)
        ? GetConsoleVariableFloat(TEXT("r.SecondaryScreenPercentage.GameViewport"), Settings->SecondaryScreenPercentageWhenNative)
        : Settings->SecondaryScreenPercentageWhenNative;
    Status.bUpscalerSafeUIEnabled =
        (!Status.ProjectCapabilities.bCapabilityScanComplete || Status.ProjectCapabilities.bSupportsUpscalerSafeUI) &&
        Status.SecondaryScreenPercentage > 0.0f;
    Status.TexturePoolBudgetMB = (!Status.ProjectCapabilities.bCapabilityScanComplete || Status.ProjectCapabilities.bSupportsTexturePoolBudgeting)
        ? GetConsoleVariableInt(TEXT("r.Streaming.PoolSize"), Status.TexturePoolBudgetMB)
        : Status.TexturePoolBudgetMB;
    Status.bVendorPolicyApplied =
        Settings->bEnableVendorAndVRAMPolicy &&
        (!Status.ProjectCapabilities.bCapabilityScanComplete || Status.ProjectCapabilities.bSupportsTexturePoolBudgeting);
    Status.bTelemetryEnabled = Settings->bEnableCsvTelemetry || Settings->MaxRecentEvents > 0;
    Status.bFireAndForgetShippingEnabled = IsFireAndForgetShippingEnabled();

    if (const UGameUserSettings* GameUserSettings = ResolveUserSettings())
    {
        Status.bDynamicResolutionManaged = GameUserSettings->IsDynamicResolutionEnabled();
    }
    else
    {
        Status.bDynamicResolutionManaged = false;
    }
}

void UTargetFrameCoreSubsystem::UpdateSetupSummary()
{
    const FString TierName = TargetFrameCorePrivate::TierToString(Status.Hardware.RecommendedTier);
    const FString VendorName = TargetFrameCorePrivate::VendorToString(Status.Hardware.GPUVendor);
    const int32 GraphicsMemoryMB =
        Status.Hardware.DeviceWorkingMemoryMB > 0 ? Status.Hardware.DeviceWorkingMemoryMB : Status.Hardware.TotalGraphicsMemoryMB;

    Status.RecommendedProfile = TierName;
    Status.SetupNotes.Reset();

    if (!VendorName.IsEmpty())
    {
        Status.SetupNotes.Add(FString::Printf(
            TEXT("%s GPU detected (%s)."),
            *VendorName,
            *Status.Hardware.GPUBrand));
    }

    if (GraphicsMemoryMB > 0)
    {
        Status.SetupNotes.Add(FString::Printf(
            TEXT("Graphics memory budget detected: %.1f GB."),
            static_cast<float>(GraphicsMemoryMB) / 1024.0f));
    }

    if (Status.bLowVRAMMode)
    {
        Status.SetupNotes.Add(TEXT("Low-VRAM safeguards are active."));
    }

    if (Status.bDynamicResolutionManaged)
    {
        Status.SetupNotes.Add(TEXT("Dynamic resolution is enabled to defend the frame budget."));
    }

    if (Status.bUpscalerSafeUIEnabled)
    {
        Status.SetupNotes.Add(FString::Printf(
            TEXT("Secondary screen percentage is %.0f%% to keep UI crisp while the scene upscales."),
            Status.SecondaryScreenPercentage));
    }
    if (Status.TexturePoolBudgetMB > 0)
    {
        Status.SetupNotes.Add(FString::Printf(
            TEXT("Texture pool budget is %d MB."),
            Status.TexturePoolBudgetMB));
    }

    if (Status.bFireAndForgetShippingEnabled)
    {
        Status.SetupNotes.Add(TEXT("Fire-and-forget shipping capsule is active."));
    }

    if (Status.bPolicyLockEngaged)
    {
        Status.SetupNotes.Add(TEXT("Policy lock is engaged after stabilization."));
    }

    if (Status.ProjectCapabilities.bCapabilityScanComplete && !Status.ProjectCapabilities.Summary.IsEmpty())
    {
        Status.SetupNotes.Add(Status.ProjectCapabilities.Summary);
    }

    Status.SetupNotes.Add(FString::Printf(
        TEXT("Nanite target is %.2f px/edge with a %.1f ms raster budget."),
        Status.NaniteMaxPixelsPerEdge,
        Status.NaniteTimeBudgetMs));

    Status.SetupSummary = FString::Printf(
        TEXT("%s profile targeting %d FPS on %s hardware with %s%s."),
        *TierName,
        Status.TargetFPS,
        *VendorName,
        Status.bLowVRAMMode ? TEXT("low-VRAM protections") : TEXT("balanced safeguards"),
        Status.bFireAndForgetShippingEnabled ? TEXT("; fire-and-forget shipping capsule active") : TEXT(""));
}

void UTargetFrameCoreSubsystem::RecordTelemetrySnapshot() const
{
    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    if (!Settings->bEnableCsvTelemetry)
    {
        return;
    }

    CSV_CUSTOM_STAT(TargetFrameCore, TargetFPS, Status.TargetFPS, ECsvCustomStatOp::Set);
    CSV_CUSTOM_STAT(TargetFrameCore, SmoothedFPS, Status.SmoothedFPS, ECsvCustomStatOp::Set);
    CSV_CUSTOM_STAT(TargetFrameCore, OverallQualityLevel, Status.OverallQualityLevel, ECsvCustomStatOp::Set);
    CSV_CUSTOM_STAT(TargetFrameCore, ResolutionScale, Status.ResolutionScale, ECsvCustomStatOp::Set);
    CSV_CUSTOM_STAT(TargetFrameCore, SecondaryScreenPercentage, Status.SecondaryScreenPercentage, ECsvCustomStatOp::Set);
    CSV_CUSTOM_STAT(TargetFrameCore, NaniteMaxPixelsPerEdge, Status.NaniteMaxPixelsPerEdge, ECsvCustomStatOp::Set);
    CSV_CUSTOM_STAT(TargetFrameCore, NaniteTimeBudgetMs, Status.NaniteTimeBudgetMs, ECsvCustomStatOp::Set);
    CSV_CUSTOM_STAT(TargetFrameCore, TexturePoolBudgetMB, Status.TexturePoolBudgetMB, ECsvCustomStatOp::Set);
    CSV_CUSTOM_STAT(TargetFrameCore, DeviceWorkingMemoryMB, Status.Hardware.DeviceWorkingMemoryMB, ECsvCustomStatOp::Set);
    CSV_CUSTOM_STAT(TargetFrameCore, GPUBenchmarkScore, Status.LastGPUBenchmarkScore, ECsvCustomStatOp::Set);
    CSV_CUSTOM_STAT(TargetFrameCore, CPUBenchmarkScore, Status.LastCPUBenchmarkScore, ECsvCustomStatOp::Set);
    CSV_CUSTOM_STAT(TargetFrameCore, PolicyApplications, Status.PolicyApplications, ECsvCustomStatOp::Set);
    CSV_CUSTOM_STAT(TargetFrameCore, FireAndForgetShipping, Status.bFireAndForgetShippingEnabled ? 1 : 0, ECsvCustomStatOp::Set);
    CSV_CUSTOM_STAT(TargetFrameCore, PolicyLockEngaged, Status.bPolicyLockEngaged ? 1 : 0, ECsvCustomStatOp::Set);
}

void UTargetFrameCoreSubsystem::AddRecentEvent(const FString& EventText)
{
    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    if (Settings->MaxRecentEvents <= 0)
    {
        return;
    }

    Status.RecentEvents.Insert(FString::Printf(TEXT("[%.1fs] %s"), ElapsedSeconds, *EventText), 0);

    if (Status.RecentEvents.Num() > Settings->MaxRecentEvents)
    {
        Status.RecentEvents.SetNum(Settings->MaxRecentEvents);
    }
}

void UTargetFrameCoreSubsystem::RecordPolicyAction(const FString& Action, const FString& Reason)
{
    Status.LastPolicyAction = Action;
    Status.LastPolicyReason = Reason;
    ++Status.PolicyApplications;

    AddRecentEvent(FString::Printf(TEXT("%s %s"), *Action, *Reason));

    if (GetDefault<UTargetFrameCoreDeveloperSettings>()->bLogAdjustments)
    {
        UE_LOG(LogTargetFrameCore, Log, TEXT("TargetFrameCore: %s %s"), *Action, *Reason);
    }

    LogRuntimeProbe(*Action);
}

void UTargetFrameCoreSubsystem::LogRuntimeProbe(const TCHAR* Context) const
{
    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    if (!Settings->bLogRuntimeProbe)
    {
        return;
    }

    const float DynamicResMinScreenPercentage = GetConsoleVariableFloat(TEXT("r.DynamicRes.MinScreenPercentage"), 0.0f);
    const float DynamicResMaxScreenPercentage = GetConsoleVariableFloat(TEXT("r.DynamicRes.MaxScreenPercentage"), 0.0f);
    const float DynamicResFrameBudgetMs = GetConsoleVariableFloat(TEXT("r.DynamicRes.FrameTimeBudget"), 0.0f);
    const int32 DynamicResOperationMode = GetConsoleVariableInt(TEXT("r.DynamicRes.OperationMode"), 0);
    const float NaniteMaxPixelsPerEdge = GetConsoleVariableFloat(TEXT("r.Nanite.MaxPixelsPerEdge"), Status.NaniteMaxPixelsPerEdge);
    const float NaniteTimeBudgetMs = GetConsoleVariableFloat(TEXT("r.Nanite.PrimaryRaster.TimeBudgetMs"), Status.NaniteTimeBudgetMs);
    const int32 NaniteTessellation = GetConsoleVariableInt(TEXT("r.Nanite.Tessellation"), Status.bNaniteTessellationAllowed ? 1 : 0);
    const float SecondaryScreenPercentage = GetConsoleVariableFloat(TEXT("r.SecondaryScreenPercentage.GameViewport"), Status.SecondaryScreenPercentage);
    const int32 TexturePoolSizeMB = GetConsoleVariableInt(TEXT("r.Streaming.PoolSize"), Status.TexturePoolBudgetMB);
    const int32 LimitPoolToVRAM = GetConsoleVariableInt(TEXT("r.Streaming.LimitPoolSizeToVRAM"), 0);
}

ETargetFrameCoreGPUVendor UTargetFrameCoreSubsystem::DetectGPUVendor(const FString& GPUBrand) const
{
    const FString NormalizedBrand = GPUBrand.ToLower();
    if (NormalizedBrand.Contains(TEXT("nvidia")) || NormalizedBrand.Contains(TEXT("geforce")) || NormalizedBrand.Contains(TEXT("quadro")))
    {
        return ETargetFrameCoreGPUVendor::NVIDIA;
    }

    if (NormalizedBrand.Contains(TEXT("amd")) || NormalizedBrand.Contains(TEXT("radeon")) || NormalizedBrand.Contains(TEXT("firepro")))
    {
        return ETargetFrameCoreGPUVendor::AMD;
    }

    if (NormalizedBrand.Contains(TEXT("intel")) || NormalizedBrand.Contains(TEXT("arc")) || NormalizedBrand.Contains(TEXT("iris")) || NormalizedBrand.Contains(TEXT("uhd")))
    {
        return ETargetFrameCoreGPUVendor::Intel;
    }

    return ETargetFrameCoreGPUVendor::Unknown;
}

ETargetFrameCoreHardwareTier UTargetFrameCoreSubsystem::DetermineHardwareTier(const FTargetFrameCoreHardwareSnapshot& Snapshot) const
{
    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    const int32 GraphicsMemoryMB = Snapshot.DeviceWorkingMemoryMB > 0 ? Snapshot.DeviceWorkingMemoryMB : Snapshot.TotalGraphicsMemoryMB;
    const bool bHasGPUScore = Status.LastGPUBenchmarkScore >= 0.0f;

    if (Snapshot.bIsIntegratedGPU)
    {
        return ETargetFrameCoreHardwareTier::Entry;
    }

    if ((GraphicsMemoryMB > 0 && GraphicsMemoryMB <= Settings->LowVRAMThresholdGB * 1024) ||
        (bHasGPUScore && Status.LastGPUBenchmarkScore < Settings->MinGPUScoreForMainstreamTier))
    {
        return ETargetFrameCoreHardwareTier::Entry;
    }

    if ((GraphicsMemoryMB > 0 && GraphicsMemoryMB < Settings->MainstreamVRAMThresholdGB * 1024) ||
        (bHasGPUScore && Status.LastGPUBenchmarkScore < Settings->MinGPUScoreForPerformanceTier))
    {
        return ETargetFrameCoreHardwareTier::Mainstream;
    }

    return ETargetFrameCoreHardwareTier::Performance;
}

int32 UTargetFrameCoreSubsystem::GetRecommendedTargetFPSForCurrentHardware() const
{
    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();

    switch (Status.Hardware.RecommendedTier)
    {
    case ETargetFrameCoreHardwareTier::Entry:
        return Settings->EntryTierTargetFPS;
    case ETargetFrameCoreHardwareTier::Mainstream:
        return Settings->MainstreamTierTargetFPS;
    case ETargetFrameCoreHardwareTier::Performance:
        return Settings->PerformanceTierTargetFPS;
    default:
        return Settings->DefaultTargetFPS;
    }
}

bool UTargetFrameCoreSubsystem::CanIncreaseQualityFurther() const
{
    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();

    if (Settings->bAdjustResolutionScale &&
        (!Status.ProjectCapabilities.bCapabilityScanComplete || Status.ProjectCapabilities.bSupportsResolutionScale) &&
        Status.ResolutionScale + KINDA_SMALL_NUMBER < GetEffectiveMaxResolutionScale())
    {
        return true;
    }
if (Settings->bAdjustOverallQuality &&
        (!Status.ProjectCapabilities.bCapabilityScanComplete || Status.ProjectCapabilities.bSupportsOverallQuality))
    {
        int32 CurrentQuality = Status.OverallQualityLevel;
        if (CurrentQuality < 0)
        {
            CurrentQuality = Settings->FallbackOverallQualityLevelWhenCustom;
        }

        if (CurrentQuality < GetEffectiveMaxOverallQualityLevel())
        {
            return true;
        }
    }

    return false;
}

bool UTargetFrameCoreSubsystem::CanReduceCostFurther() const
{
    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();

    if (Settings->bAdjustOverallQuality &&
        (!Status.ProjectCapabilities.bCapabilityScanComplete || Status.ProjectCapabilities.bSupportsOverallQuality))
    {
        int32 CurrentQuality = Status.OverallQualityLevel;
        if (CurrentQuality < 0)
        {
            CurrentQuality = Settings->FallbackOverallQualityLevelWhenCustom;
        }

        if (CurrentQuality > Settings->MinOverallQualityLevel)
        {
            return true;
        }
    }
if (Settings->bAdjustResolutionScale &&
        (!Status.ProjectCapabilities.bCapabilityScanComplete || Status.ProjectCapabilities.bSupportsResolutionScale) &&
        Status.ResolutionScale > Settings->MinResolutionScale + KINDA_SMALL_NUMBER)
    {
        return true;
    }

    return false;
}

int32 UTargetFrameCoreSubsystem::GetEffectiveMaxOverallQualityLevel() const
{
    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    int32 EffectiveMaxQuality = Settings->MaxOverallQualityLevel;

    if (!Settings->bEnableVendorAndVRAMPolicy)
    {
        return EffectiveMaxQuality;
    }

    if (Status.bLowVRAMMode)
    {
        EffectiveMaxQuality = FMath::Min(EffectiveMaxQuality, Settings->MaxQualityLevelInLowVRAMMode);
    }

    if (Status.Hardware.GPUVendor == ETargetFrameCoreGPUVendor::Intel)
    {
        EffectiveMaxQuality = FMath::Min(EffectiveMaxQuality, Settings->MaxQualityLevelOnIntel);
    }

    return FMath::Max(Settings->MinOverallQualityLevel, EffectiveMaxQuality);
}

float UTargetFrameCoreSubsystem::GetEffectiveMaxResolutionScale() const
{
    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();

    if (!Settings->bEnableVendorAndVRAMPolicy || !Status.bLowVRAMMode)
    {
        return Settings->MaxResolutionScale;
    }

    return FMath::Min(Settings->MaxResolutionScale, Settings->MaxResolutionScaleInLowVRAMMode);
}

bool UTargetFrameCoreSubsystem::ShouldUseLowVRAMMode() const
{
    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    const int32 GraphicsMemoryMB =
        Status.Hardware.DeviceWorkingMemoryMB > 0 ? Status.Hardware.DeviceWorkingMemoryMB : Status.Hardware.TotalGraphicsMemoryMB;

    return Status.Hardware.bIsIntegratedGPU ||
        (GraphicsMemoryMB > 0 && GraphicsMemoryMB <= Settings->LowVRAMThresholdGB * 1024);
}

bool UTargetFrameCoreSubsystem::ShouldForceDynamicResolution() const
{
    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    if (!Settings->bEnableVendorAndVRAMPolicy ||
        (Status.ProjectCapabilities.bCapabilityScanComplete && !Status.ProjectCapabilities.bSupportsDynamicResolution))
    {
        return false;
    }

    return Status.bLowVRAMMode ||
        (Settings->bPreferDynamicResolutionOnIntel && Status.Hardware.GPUVendor == ETargetFrameCoreGPUVendor::Intel);
}

bool UTargetFrameCoreSubsystem::IsFireAndForgetShippingEnabled() const
{
    if (bFireAndForgetShippingOverrideSet)
    {
        return bFireAndForgetShippingOverrideValue;
    }

    return GetDefault<UTargetFrameCoreDeveloperSettings>()->bEnableFireAndForgetShipping;
}

bool UTargetFrameCoreSubsystem::DoesConsoleVariableExist(const TCHAR* Name) const
{
    return IConsoleManager::Get().FindConsoleVariable(Name) != nullptr;
}

int32 UTargetFrameCoreSubsystem::GetConsoleVariableInt(const TCHAR* Name, int32 DefaultValue) const
{
    if (const IConsoleVariable* ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(Name))
    {
        return ConsoleVariable->GetInt();
    }

    return DefaultValue;
}

float UTargetFrameCoreSubsystem::GetConsoleVariableFloat(const TCHAR* Name, float DefaultValue) const
{
    if (const IConsoleVariable* ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(Name))
    {
        return ConsoleVariable->GetFloat();
    }

    return DefaultValue;
}

void UTargetFrameCoreSubsystem::SetConsoleVariableInt(const TCHAR* Name, int32 Value) const
{
    if (IConsoleVariable* ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(Name))
    {
        ConsoleVariable->Set(Value, ECVF_SetByGameSetting);
    }
}

void UTargetFrameCoreSubsystem::SetConsoleVariableFloat(const TCHAR* Name, float Value) const
{
    if (IConsoleVariable* ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(Name))
    {
        ConsoleVariable->Set(Value, ECVF_SetByGameSetting);
    }
}

void UTargetFrameCoreSubsystem::ConfigureFireAndForgetShipping()
{
    const UTargetFrameCoreDeveloperSettings* Settings = GetDefault<UTargetFrameCoreDeveloperSettings>();
    Status.bFireAndForgetShippingEnabled = IsFireAndForgetShippingEnabled();

    if (!Status.bFireAndForgetShippingEnabled)
    {
        return;
    }

    if (Settings->bProbeProjectCapabilities && !Status.ProjectCapabilities.bCapabilityScanComplete)
    {
        ProbeProjectCapabilities();
    }

    if (Settings->bAutoSelectTargetFPSByHardwareTier)
    {
        const int32 RecommendedTargetFPS = FMath::Clamp(GetRecommendedTargetFPSForCurrentHardware(), 15, 240);
        if (RecommendedTargetFPS != Status.TargetFPS)
        {
            Status.TargetFPS = RecommendedTargetFPS;
            AddRecentEvent(FString::Printf(
                TEXT("Fire-and-forget shipping selected %d FPS for the %s tier."),
                Status.TargetFPS,
                *TargetFrameCorePrivate::TierToString(Status.Hardware.RecommendedTier)));
        }

        UE_LOG(
            LogTargetFrameCore,
            Log,
            TEXT("TargetFrameCore fire-and-forget shipping configured target FPS %d for %s tier. %s"),
            RecommendedTargetFPS,
            *TargetFrameCorePrivate::TierToString(Status.Hardware.RecommendedTier),
            Status.ProjectCapabilities.Summary.IsEmpty() ? TEXT("No capability summary.") : *Status.ProjectCapabilities.Summary);
    }

    UpdateSetupSummary();
}

UGameUserSettings* UTargetFrameCoreSubsystem::ResolveUserSettings() const
{
    return GEngine != nullptr ? GEngine->GetGameUserSettings() : nullptr;
}
