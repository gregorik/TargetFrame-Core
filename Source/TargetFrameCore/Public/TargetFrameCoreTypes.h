// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "TargetFrameCoreTypes.generated.h"

UENUM(BlueprintType)
enum class ETargetFrameCoreGPUVendor : uint8
{
    Unknown,
    NVIDIA,
    AMD,
    Intel
};

UENUM(BlueprintType)
enum class ETargetFrameCoreHardwareTier : uint8
{
    Unknown,
    Entry,
    Mainstream,
    Performance
};

USTRUCT(BlueprintType)
struct TARGETFRAMECORE_API FTargetFrameCoreHardwareSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    FString PlatformName;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    FString RHIName;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    FString GPUBrand;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    ETargetFrameCoreGPUVendor GPUVendor = ETargetFrameCoreGPUVendor::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    ETargetFrameCoreHardwareTier RecommendedTier = ETargetFrameCoreHardwareTier::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 LogicalCoreCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 SystemMemoryGB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 DeviceWorkingMemoryMB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 DedicatedVideoMemoryMB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 TotalGraphicsMemoryMB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 SharedSystemMemoryMB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 TexturePoolSizeMB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 StreamingMemorySizeMB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 AvailableStreamingMemoryMB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bSupportsRayTracing = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bIsIntegratedGPU = false;
};

USTRUCT(BlueprintType)
struct TARGETFRAMECORE_API FTargetFrameCoreProjectCapabilities
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bCapabilityScanComplete = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bSupportsGameUserSettings = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bSupportsOverallQuality = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bSupportsResolutionScale = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bSupportsDynamicResolution = false;
    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bSupportsUpscalerSafeUI = false;
    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bSupportsTexturePoolBudgeting = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    FString Summary;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    TArray<FString> Notes;
};

USTRUCT(BlueprintType)
struct TARGETFRAMECORE_API FTargetFrameCoreSetupSummary
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    FString ProfileName;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    FString Summary;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 RecommendedTargetFPS = 60;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bLowVRAMMode = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bDynamicResolutionRecommended = false;
    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bFireAndForgetShippingEnabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bPolicyLockEngaged = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    float SecondaryScreenPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    TArray<FString> Notes;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    FTargetFrameCoreProjectCapabilities ProjectCapabilities;
};

USTRUCT(BlueprintType)
struct TARGETFRAMECORE_API FTargetFrameCoreStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bBenchmarkApplied = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 TargetFPS = 60;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    float SmoothedFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 ConsecutiveLowSamples = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 ConsecutiveHighSamples = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 OverallQualityLevel = 3;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    float ResolutionScale = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    float LastCPUBenchmarkScore = -1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    float LastGPUBenchmarkScore = -1.0f;
    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bLowVRAMMode = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bVendorPolicyApplied = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bUpscalerSafeUIEnabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bTelemetryEnabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bDynamicResolutionManaged = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bNaniteTessellationAllowed = true;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bFireAndForgetShippingEnabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    bool bPolicyLockEngaged = false;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 StableEvaluations = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    float SecondaryScreenPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    float NaniteMaxPixelsPerEdge = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    float NaniteTimeBudgetMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 TexturePoolBudgetMB = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    int32 PolicyApplications = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    FString RecommendedProfile;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    FString SetupSummary;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    FString LastPolicyAction;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    FString LastPolicyReason;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    TArray<FString> SetupNotes;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    TArray<FString> RecentEvents;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    FTargetFrameCoreHardwareSnapshot Hardware;

    UPROPERTY(BlueprintReadOnly, Category = "TargetFrameCore")
    FTargetFrameCoreProjectCapabilities ProjectCapabilities;
};
