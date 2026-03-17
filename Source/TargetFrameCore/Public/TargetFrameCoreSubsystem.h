#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "TargetFrameCoreTypes.h"
#include "TargetFrameCoreSubsystem.generated.h"

class UGameUserSettings;

UCLASS()
class TARGETFRAMECORE_API UTargetFrameCoreSubsystem final : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    UTargetFrameCoreSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;
    virtual bool IsTickableInEditor() const override;
    virtual bool IsTickableWhenPaused() const override;

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore")
    void StartAutomaticSetup(bool bForceRebenchmark = false);

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore")
    void SetTargetFPS(int32 NewTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore")
    void ApplyCurrentPolicy();

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore")
    void SetFireAndForgetShippingEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "TargetFrameCore")
    FTargetFrameCoreStatus GetStatus() const;

    UFUNCTION(BlueprintPure, Category = "TargetFrameCore")
    FTargetFrameCoreSetupSummary GetSetupSummary() const;

    UFUNCTION(BlueprintPure, Category = "TargetFrameCore")
    FTargetFrameCoreProjectCapabilities GetProjectCapabilities() const;

private:
    void RefreshHardwareSnapshot();
    void ProbeProjectCapabilities();
    void RunHardwareBenchmark();
    void LoadStateFromGameUserSettings();
    void SaveCurrentSettings();
    void ConfigureFireAndForgetShipping();
    void EvaluateRuntimeBudget();
    void StepQualityDown();
    void StepQualityUp();
    void ApplyVendorAndMemoryPolicy();
        void ApplyUISeparationPolicy(bool bDynamicResolutionEnabled);
    bool TryAdjustResolutionScale(float DeltaScale);
    bool TryAdjustOverallQuality(int32 DeltaLevel);
            void RefreshRuntimeStatusFromConsoleVariables();
    void UpdateSetupSummary();
    void RecordTelemetrySnapshot() const;
    void AddRecentEvent(const FString& EventText);
    void RecordPolicyAction(const FString& Action, const FString& Reason);
    void LogRuntimeProbe(const TCHAR* Context) const;
    ETargetFrameCoreGPUVendor DetectGPUVendor(const FString& GPUBrand) const;
    ETargetFrameCoreHardwareTier DetermineHardwareTier(const FTargetFrameCoreHardwareSnapshot& Snapshot) const;
            int32 GetRecommendedTargetFPSForCurrentHardware() const;
    bool CanIncreaseQualityFurther() const;
    bool CanReduceCostFurther() const;
    int32 GetEffectiveMaxOverallQualityLevel() const;
    float GetEffectiveMaxResolutionScale() const;
    bool ShouldUseLowVRAMMode() const;
    bool ShouldForceDynamicResolution() const;
    bool IsFireAndForgetShippingEnabled() const;
    bool DoesConsoleVariableExist(const TCHAR* Name) const;
    int32 GetConsoleVariableInt(const TCHAR* Name, int32 DefaultValue) const;
    float GetConsoleVariableFloat(const TCHAR* Name, float DefaultValue) const;
    void SetConsoleVariableInt(const TCHAR* Name, int32 Value) const;
    void SetConsoleVariableFloat(const TCHAR* Name, float Value) const;
    UGameUserSettings* ResolveUserSettings() const;

    bool bSubsystemInitialized = false;
    float ElapsedSeconds = 0.0f;
    float SecondsSinceLastEvaluation = 0.0f;
    float SecondsSinceLastRuntimeProbe = 0.0f;
    bool bFireAndForgetShippingOverrideSet = false;
    bool bFireAndForgetShippingOverrideValue = false;
    FTargetFrameCoreStatus Status;
};
