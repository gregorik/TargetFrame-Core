#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TargetFrameCoreUserExperienceWidget.generated.h"

class APlayerController;
class UBorder;
class UButton;
class UCheckBox;
class UTextBlock;
class UVerticalBox;

UCLASS(BlueprintType, Blueprintable)
class TARGETFRAMECORE_API UTargetFrameCoreUserExperienceWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;
    virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore|UX")
    void InitializeExperience(APlayerController* InPlayerController, bool bShowOnboarding);

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore|UX")
    void ToggleControlPanel();

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore|UX")
    void ShowControlPanel();

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore|UX")
    void HideControlPanel();

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore|UX")
    void ShowOnboarding();

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore|UX")
    void RefreshPresentation();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Branding")
    FString BrandLabel = TEXT("GregOrigin");

    UPROPERTY(EditDefaultsOnly, Category = "TargetFrameCore|UX", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float RefreshIntervalSeconds = 1.0f;

private:
    void ApplyVisualStyle();
    void ApplyInputMode() const;
    void UpdateShowcaseFocus() const;
    void SetControlPanelVisible(bool bVisible);
    void SetOnboardingVisible(bool bVisible);
    void SetTargetFPSAndApply(int32 NewTargetFPS);
    void RebuildTextList(UVerticalBox* ListWidget, const TArray<FString>& Entries, int32 MaxEntries, const FLinearColor& EntryColor);
    UTextBlock* AddListEntry(UVerticalBox* ListWidget, const FString& Text, const FLinearColor& EntryColor);

    UFUNCTION()
    void HandleOpenLauncher();

    UFUNCTION()
    void HandleClosePanel();

    UFUNCTION()
    void HandleApplyPolicy();

    UFUNCTION()
    void HandleRebenchmark();

    UFUNCTION()
    void HandlePreset30();

    UFUNCTION()
    void HandlePreset45();

    UFUNCTION()
    void HandlePreset60();

    UFUNCTION()
    void HandlePreset90();

    UFUNCTION()
    void HandleRunGuidedSetup();

    UFUNCTION()
    void HandleOpenControlPanelFromOnboarding();

    UFUNCTION()
    void HandleSkipOnboarding();

    UFUNCTION()
    void HandleRememberOnboardingChanged(bool bIsChecked);

    TWeakObjectPtr<APlayerController> CachedPlayerController;
    bool bControlPanelVisible = false;
    bool bOnboardingVisible = false;
    bool bRememberOnboarding = true;
    float RefreshAccumulator = 0.0f;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> LauncherButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> ScrimBorder = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> PanelBorder = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> OnboardingBorder = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> HeaderCard = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> HardwarePillBorder = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> ControlsCard = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> OverviewCard = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> RenderingCard = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> HardwareCard = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> ActionCard = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> ShippingCapsuleCard = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> NotesCard = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> EventsCard = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> OnboardingHardwareCard = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> OnboardingRecommendationCard = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> HeaderTitleText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> HeaderSummaryText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> HeaderHardwarePillText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> LauncherLabelText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> OverviewValueText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> RenderingValueText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> HardwareValueText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ActionValueText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ShippingCapsuleValueText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> SetupNotesList = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> RecentEventsList = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> OnboardingTitleText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> OnboardingBodyText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> OnboardingHardwareText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> OnboardingRecommendationText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UCheckBox> RememberOnboardingCheckBox = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ApplyPolicyButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> RebenchmarkButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ClosePanelButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Preset30Button = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Preset45Button = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Preset60Button = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Preset90Button = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> RunGuidedSetupButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> OpenControlPanelButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> SkipOnboardingButton = nullptr;
};
