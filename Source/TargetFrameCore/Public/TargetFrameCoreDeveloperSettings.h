// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TargetFrameCoreDeveloperSettings.generated.h"

UCLASS(Config = TargetFrameCore, DefaultConfig, meta = (DisplayName = "TargetFrameCore"))
class TARGETFRAMECORE_API UTargetFrameCoreDeveloperSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UTargetFrameCoreDeveloperSettings();

    virtual FName GetCategoryName() const override;
    virtual FText GetSectionText() const override;
    virtual FText GetSectionDescription() const override;

    UPROPERTY(Config, EditAnywhere, Category = "Bootstrap", meta = (ClampMin = "15", ClampMax = "240"))
    int32 DefaultTargetFPS = 60;
    bool bAutoRunHardwareBenchmarkOnInitialize = true;
    bool bAutoApplyHardwareBenchmarkResults = true;
    bool bEnableFireAndForgetShipping = false;
    bool bProbeProjectCapabilities = true;
    bool bAutoSelectTargetFPSByHardwareTier = true;

    UPROPERTY(Config, EditAnywhere, Category = "Fire-and-Forget Shipping", meta = (ClampMin = "15", ClampMax = "240"))
    int32 EntryTierTargetFPS = 45;

    UPROPERTY(Config, EditAnywhere, Category = "Fire-and-Forget Shipping", meta = (ClampMin = "15", ClampMax = "240"))
    int32 MainstreamTierTargetFPS = 60;

    UPROPERTY(Config, EditAnywhere, Category = "Fire-and-Forget Shipping", meta = (ClampMin = "15", ClampMax = "240"))
    int32 PerformanceTierTargetFPS = 90;
    bool bLockPolicyAfterStabilization = true;

    UPROPERTY(Config, EditAnywhere, Category = "Fire-and-Forget Shipping", meta = (ClampMin = "1", ClampMax = "20"))
    int32 StableEvaluationsBeforeLock = 4;

    UPROPERTY(Config, EditAnywhere, Category = "Fire-and-Forget Shipping", meta = (ClampMin = "0.0"))
    float SevereDeficitFPSBeforeUnlock = 12.0f;
    bool bEnableRuntimeGovernor = true;

    UPROPERTY(Config, EditAnywhere, Category = "Runtime Governor", meta = (ClampMin = "0.0"))
    float WarmupSeconds = 10.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Runtime Governor", meta = (ClampMin = "0.5"))
    float EvaluationIntervalSeconds = 4.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Runtime Governor", meta = (ClampMin = "0.0"))
    float AllowedDeficitFPS = 3.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Runtime Governor", meta = (ClampMin = "0.0"))
    float RecoveryHeadroomFPS = 8.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Runtime Governor", meta = (ClampMin = "1", ClampMax = "20"))
    int32 ConsecutiveLowSamplesRequired = 2;

    UPROPERTY(Config, EditAnywhere, Category = "Runtime Governor", meta = (ClampMin = "1", ClampMax = "20"))
    int32 ConsecutiveHighSamplesRequired = 3;
    bool bAdjustOverallQuality = true;

    UPROPERTY(Config, EditAnywhere, Category = "Quality Steps", meta = (ClampMin = "0", ClampMax = "4"))
    int32 FallbackOverallQualityLevelWhenCustom = 2;

    UPROPERTY(Config, EditAnywhere, Category = "Quality Steps", meta = (ClampMin = "0", ClampMax = "4"))
    int32 MinOverallQualityLevel = 0;

    UPROPERTY(Config, EditAnywhere, Category = "Quality Steps", meta = (ClampMin = "0", ClampMax = "4"))
    int32 MaxOverallQualityLevel = 3;
    bool bAdjustResolutionScale = true;

    UPROPERTY(Config, EditAnywhere, Category = "Resolution", meta = (ClampMin = "10.0", ClampMax = "100.0"))
    float MinResolutionScale = 67.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Resolution", meta = (ClampMin = "10.0", ClampMax = "200.0"))
    float MaxResolutionScale = 100.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Resolution", meta = (ClampMin = "1.0", ClampMax = "50.0"))
    float ResolutionScaleStep = 5.0f;
    bool bEnableDynamicResolution = false;

    UPROPERTY(Config, EditAnywhere, Category = "Resolution", meta = (ClampMin = "10.0", ClampMax = "100.0"))
    float DynamicResolutionMinScreenPercentage = 67.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Resolution", meta = (ClampMin = "10.0", ClampMax = "200.0"))
    float DynamicResolutionMaxScreenPercentage = 100.0f;
    bool bEnableVendorAndVRAMPolicy = true;

    UPROPERTY(Config, EditAnywhere, Category = "Vendor and Memory Policy", meta = (ClampMin = "2", ClampMax = "32"))
    int32 LowVRAMThresholdGB = 8;

    UPROPERTY(Config, EditAnywhere, Category = "Vendor and Memory Policy", meta = (ClampMin = "2", ClampMax = "32"))
    int32 MainstreamVRAMThresholdGB = 12;

    UPROPERTY(Config, EditAnywhere, Category = "Vendor and Memory Policy", meta = (ClampMin = "0.0"))
    float MinGPUScoreForMainstreamTier = 130.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Vendor and Memory Policy", meta = (ClampMin = "0.0"))
    float MinGPUScoreForPerformanceTier = 220.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Vendor and Memory Policy", meta = (ClampMin = "0", ClampMax = "4"))
    int32 MaxQualityLevelOnIntel = 2;

    UPROPERTY(Config, EditAnywhere, Category = "Vendor and Memory Policy", meta = (ClampMin = "0", ClampMax = "4"))
    int32 MaxQualityLevelInLowVRAMMode = 2;

    UPROPERTY(Config, EditAnywhere, Category = "Vendor and Memory Policy", meta = (ClampMin = "10.0", ClampMax = "100.0"))
    float MaxResolutionScaleInLowVRAMMode = 90.0f;
    bool bPreferDynamicResolutionOnIntel = true;
    bool bLimitTexturePoolToVRAM = true;

    UPROPERTY(Config, EditAnywhere, Category = "Vendor and Memory Policy", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float TexturePoolBudgetFraction = 0.72f;

    UPROPERTY(Config, EditAnywhere, Category = "Vendor and Memory Policy", meta = (ClampMin = "128"))
    int32 MinimumTexturePoolSizeMB = 768;
    bool bEnableUpscalerSafeUI = true;
    bool bOnlySeparateUIWhenUpscaling = true;

    UPROPERTY(Config, EditAnywhere, Category = "Upscaler and UI", meta = (ClampMin = "0.0", ClampMax = "200.0"))
    float SecondaryScreenPercentageWhenNative = 0.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Upscaler and UI", meta = (ClampMin = "10.0", ClampMax = "200.0"))
    float SecondaryScreenPercentageForUI = 100.0f;
    bool bEnableCsvTelemetry = true;

    UPROPERTY(Config, EditAnywhere, Category = "Telemetry", meta = (ClampMin = "0", ClampMax = "32"))
    int32 MaxRecentEvents = 8;
    bool bLogRuntimeProbe = true;

    UPROPERTY(Config, EditAnywhere, Category = "Diagnostics", meta = (ClampMin = "1.0", ClampMax = "120.0"))
    float RuntimeProbeIntervalSeconds = 10.0f;
    bool bLogAdjustments = true;
};
