// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#include "TargetFrameCoreUserExperienceWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "TargetFrameCoreBlueprintLibrary.h"
#include "TargetFrameCoreModule.h"
#include "TargetFrameCoreSubsystem.h"

namespace TargetFrameCoreUX
{
    const FLinearColor PanelColor(0.05f, 0.07f, 0.10f, 0.995f);
    const FLinearColor CardColor(0.08f, 0.11f, 0.16f, 1.0f);
    const FLinearColor CardElevatedColor(0.11f, 0.15f, 0.21f, 1.0f);
    const FLinearColor AccentColor(0.95f, 0.56f, 0.19f, 1.0f);
    const FLinearColor BrandColor(0.18f, 0.82f, 0.76f, 1.0f);
    const FLinearColor TextColor(0.95f, 0.97f, 1.0f, 1.0f);
    const FLinearColor MutedTextColor(0.66f, 0.72f, 0.80f, 1.0f);
    const FLinearColor PositiveColor(0.44f, 0.84f, 0.56f, 1.0f);
    const FLinearColor WarningColor(0.98f, 0.77f, 0.32f, 1.0f);
    const FLinearColor ScrimColor(0.01f, 0.02f, 0.03f, 0.84f);

    FString CompactText(const FString& Text, int32 MaxCharacters = 56)
    {
        if (Text.Len() <= MaxCharacters)
        {
            return Text;
        }

        return FString::Printf(TEXT("%s..."), *Text.Left(MaxCharacters - 3));
    }

    FString VendorLabel(ETargetFrameCoreGPUVendor Vendor)
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
            return TEXT("");
        }
    }

    FString FormatVRAMLabel(int32 WorkingMemoryMB)
    {
        if (WorkingMemoryMB >= 1024)
        {
            return FString::Printf(TEXT("%.1f GB VRAM"), static_cast<float>(WorkingMemoryMB) / 1024.0f);
        }

        return FString::Printf(TEXT("%d MB VRAM"), WorkingMemoryMB);
    }

    FString MakeHardwarePillLabel(const FTargetFrameCoreStatus& Status)
    {
        const FString GPUVendorLabel = VendorLabel(Status.Hardware.GPUVendor);
        const FString GPULabel = GPUVendorLabel.IsEmpty() ? CompactText(Status.Hardware.GPUBrand, 12) : GPUVendorLabel;
        return FString::Printf(TEXT("%s | %s"), *GPULabel, *FormatVRAMLabel(Status.Hardware.DeviceWorkingMemoryMB));
    }

    int32 CountSupportedShippingControls(const FTargetFrameCoreProjectCapabilities& Capabilities)
    {
        int32 Count = 0;
        Count += Capabilities.bSupportsOverallQuality ? 1 : 0;
        Count += Capabilities.bSupportsResolutionScale ? 1 : 0;
        Count += Capabilities.bSupportsDynamicResolution ? 1 : 0;
        Count += Capabilities.bSupportsUpscalerSafeUI ? 1 : 0;
        Count += Capabilities.bSupportsTexturePoolBudgeting ? 1 : 0;
        return Count;
    }

    FString BuildShippingControlLabel(const FTargetFrameCoreProjectCapabilities& Capabilities)
    {
        TArray<FString> Controls;

        if (Capabilities.bSupportsOverallQuality)
        {
            Controls.Add(TEXT("quality"));
        }

        if (Capabilities.bSupportsResolutionScale)
        {
            Controls.Add(TEXT("scale"));
        }

        if (Capabilities.bSupportsDynamicResolution)
        {
            Controls.Add(TEXT("dyn res"));
        }

        {
            Controls.Add(TEXT("Nanite"));
        }

        if (Capabilities.bSupportsUpscalerSafeUI)
        {
            Controls.Add(TEXT("UI"));
        }

        {
            Controls.Add(TEXT("HWRT"));
        }

        if (Capabilities.bSupportsTexturePoolBudgeting)
        {
            Controls.Add(TEXT("pool"));
        }

        return Controls.Num() > 0 ? FString::Join(Controls, TEXT(", ")) : TEXT("observation only");
    }

    FString DescribeShippingMode(const FTargetFrameCoreProjectCapabilities& Capabilities)
    {
        if (!Capabilities.bCapabilityScanComplete)
        {
            return TEXT("Scanning");
        }

        const int32 SupportedControls = CountSupportedShippingControls(Capabilities);
        if (SupportedControls <= 0)
        {
            return TEXT("Observation only");
        }

        return SupportedControls >= 7 ? TEXT("Full support") : TEXT("Compatible mode");
    }

    FSlateBrush MakeSolidBrush(const FLinearColor& Color)
    {
        FSlateColorBrush Brush(Color);
        return Brush;
    }

    FSlateBrush MakeRoundedBrush(
        const FLinearColor& Color,
        float Radius = 8.0f,
        const FLinearColor& OutlineColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.05f),
        float OutlineWidth = 1.0f)
    {
        FSlateRoundedBoxBrush Brush(Color, Radius, OutlineColor, OutlineWidth);
        return Brush;
    }

    FButtonStyle MakeButtonStyle(const FLinearColor& Normal, const FLinearColor& Hovered, const FLinearColor& Pressed)
    {
        FButtonStyle Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
        Style.SetNormal(MakeRoundedBrush(Normal, 12.0f));
        Style.SetHovered(MakeRoundedBrush(Hovered, 12.0f));
        Style.SetPressed(MakeRoundedBrush(Pressed, 12.0f));
        Style.SetNormalPadding(FMargin(12.0f, 8.0f));
        Style.SetPressedPadding(FMargin(12.0f, 9.0f, 12.0f, 7.0f));
        return Style;
    }

    UTextBlock* CreateText(UWidgetTree* WidgetTree, const FString& Text, const FLinearColor& Color, int32 Size, const TCHAR* Weight = TEXT("Regular"))
    {
        UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        TextBlock->SetText(FText::FromString(Text));
        TextBlock->SetColorAndOpacity(FSlateColor(Color));
        TextBlock->SetFont(FCoreStyle::GetDefaultFontStyle(Weight, Size));
        return TextBlock;
    }

    void StyleText(UTextBlock* TextBlock, const FLinearColor& Color, int32 Size, const TCHAR* Weight = TEXT("Regular"), float WrapTextAt = 0.0f, ETextJustify::Type Justification = ETextJustify::Left)
    {
        if (TextBlock == nullptr)
        {
            return;
        }

        TextBlock->SetColorAndOpacity(FSlateColor(Color));
        TextBlock->SetFont(FCoreStyle::GetDefaultFontStyle(Weight, Size));
        TextBlock->SetJustification(Justification);
        TextBlock->SetAutoWrapText(WrapTextAt > 0.0f);
        TextBlock->SetWrapTextAt(WrapTextAt);
    }

    void StyleBorder(UBorder* Border, const FLinearColor& Color, const FMargin& Padding, float Radius = 8.0f)
    {
        if (Border == nullptr)
        {
            return;
        }

        Border->SetBrush(MakeRoundedBrush(Color, Radius));
        Border->SetPadding(Padding);
        Border->SetClipping(EWidgetClipping::ClipToBounds);
    }

    void StyleButton(UButton* Button, const FLinearColor& Normal, const FLinearColor& Hovered, const FLinearColor& Pressed)
    {
        if (Button == nullptr)
        {
            return;
        }

        Button->SetStyle(MakeButtonStyle(Normal, Hovered, Pressed));
        Button->SetClipping(EWidgetClipping::ClipToBounds);
    }

    bool WantsBoldFallbackFont(const UTextBlock* TextBlock)
    {
        if (TextBlock == nullptr)
        {
            return false;
        }

        const FName TypefaceName = TextBlock->GetFont().TypefaceFontName;
        return TypefaceName == TEXT("Bold") || TypefaceName == TEXT("SemiBold");
    }

    void NormalizeTextBlockFont(UTextBlock* TextBlock)
    {
        if (TextBlock == nullptr)
        {
            return;
        }

        const FSlateFontInfo CurrentFont = TextBlock->GetFont();
        const int32 FontSize = FMath::Max(CurrentFont.Size, 10);
        TextBlock->SetFont(FCoreStyle::GetDefaultFontStyle(WantsBoldFallbackFont(TextBlock) ? TEXT("Bold") : TEXT("Regular"), FontSize));
    }

    void NormalizeWidgetTreeFonts(UWidgetTree* WidgetTree)
    {
        if (WidgetTree == nullptr)
        {
            return;
        }

        TArray<UWidget*> AllWidgets;
        WidgetTree->GetAllWidgets(AllWidgets);
        for (UWidget* Widget : AllWidgets)
        {
            if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
            {
                NormalizeTextBlockFont(TextBlock);
            }
        }
    }
}

void UTargetFrameCoreUserExperienceWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    ApplyVisualStyle();

    if (LauncherButton != nullptr)
    {
        LauncherButton->OnClicked.AddDynamic(this, &UTargetFrameCoreUserExperienceWidget::HandleOpenLauncher);
    }

    if (ClosePanelButton != nullptr)
    {
        ClosePanelButton->OnClicked.AddDynamic(this, &UTargetFrameCoreUserExperienceWidget::HandleClosePanel);
    }

    if (ApplyPolicyButton != nullptr)
    {
        ApplyPolicyButton->OnClicked.AddDynamic(this, &UTargetFrameCoreUserExperienceWidget::HandleApplyPolicy);
    }

    if (RebenchmarkButton != nullptr)
    {
        RebenchmarkButton->OnClicked.AddDynamic(this, &UTargetFrameCoreUserExperienceWidget::HandleRebenchmark);
    }

    if (Preset30Button != nullptr)
    {
        Preset30Button->OnClicked.AddDynamic(this, &UTargetFrameCoreUserExperienceWidget::HandlePreset30);
    }

    if (Preset45Button != nullptr)
    {
        Preset45Button->OnClicked.AddDynamic(this, &UTargetFrameCoreUserExperienceWidget::HandlePreset45);
    }

    if (Preset60Button != nullptr)
    {
        Preset60Button->OnClicked.AddDynamic(this, &UTargetFrameCoreUserExperienceWidget::HandlePreset60);
    }

    if (Preset90Button != nullptr)
    {
        Preset90Button->OnClicked.AddDynamic(this, &UTargetFrameCoreUserExperienceWidget::HandlePreset90);
    }

    if (RunGuidedSetupButton != nullptr)
    {
        RunGuidedSetupButton->OnClicked.AddDynamic(this, &UTargetFrameCoreUserExperienceWidget::HandleRunGuidedSetup);
    }

    if (OpenControlPanelButton != nullptr)
    {
        OpenControlPanelButton->OnClicked.AddDynamic(this, &UTargetFrameCoreUserExperienceWidget::HandleOpenControlPanelFromOnboarding);
    }

    if (SkipOnboardingButton != nullptr)
    {
        SkipOnboardingButton->OnClicked.AddDynamic(this, &UTargetFrameCoreUserExperienceWidget::HandleSkipOnboarding);
    }

    if (RememberOnboardingCheckBox != nullptr)
    {
        RememberOnboardingCheckBox->OnCheckStateChanged.AddDynamic(this, &UTargetFrameCoreUserExperienceWidget::HandleRememberOnboardingChanged);
    }
}

void UTargetFrameCoreUserExperienceWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
    ApplyVisualStyle();
}

void UTargetFrameCoreUserExperienceWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!CachedPlayerController.IsValid())
    {
        CachedPlayerController = GetOwningPlayer();
    }

    ApplyVisualStyle();
    RefreshPresentation();
    ApplyInputMode();
}

void UTargetFrameCoreUserExperienceWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    RefreshAccumulator += InDeltaTime;
    if (RefreshAccumulator >= RefreshIntervalSeconds)
    {
        RefreshAccumulator = 0.0f;
        RefreshPresentation();
    }
}

void UTargetFrameCoreUserExperienceWidget::InitializeExperience(APlayerController* InPlayerController, bool bShowOnboarding)
{
    CachedPlayerController = InPlayerController;
    bRememberOnboarding = true;

    if (RememberOnboardingCheckBox != nullptr)
    {
        RememberOnboardingCheckBox->SetIsChecked(bRememberOnboarding);
    }

    SetControlPanelVisible(false);
    SetOnboardingVisible(bShowOnboarding);
    RefreshPresentation();
    ApplyInputMode();

    UE_LOG(
        LogTargetFrameCore,
        Display,
        TEXT("TargetFrameCore UX initialized. Onboarding=%s ControlPanel=%s PlayerController=%s"),
        bOnboardingVisible ? TEXT("true") : TEXT("false"),
        bControlPanelVisible ? TEXT("true") : TEXT("false"),
        CachedPlayerController.IsValid() ? *CachedPlayerController->GetName() : TEXT("None"));
}

void UTargetFrameCoreUserExperienceWidget::ToggleControlPanel()
{
    if (bOnboardingVisible)
    {
        SetOnboardingVisible(false);
        SetControlPanelVisible(true);
    }
    else
    {
        SetControlPanelVisible(!bControlPanelVisible);
    }

    RefreshPresentation();
    ApplyInputMode();
}

void UTargetFrameCoreUserExperienceWidget::ShowControlPanel()
{
    SetOnboardingVisible(false);
    SetControlPanelVisible(true);
    RefreshPresentation();
    ApplyInputMode();
}

void UTargetFrameCoreUserExperienceWidget::HideControlPanel()
{
    SetControlPanelVisible(false);
    ApplyInputMode();
}

void UTargetFrameCoreUserExperienceWidget::ShowOnboarding()
{
    SetControlPanelVisible(false);
    SetOnboardingVisible(true);
    ApplyInputMode();
}

void UTargetFrameCoreUserExperienceWidget::ApplyVisualStyle()
{
    if (ScrimBorder != nullptr)
    {
        ScrimBorder->SetBrush(TargetFrameCoreUX::MakeSolidBrush(TargetFrameCoreUX::ScrimColor));
        ScrimBorder->SetPadding(FMargin(0.0f));
        ScrimBorder->SetClipping(EWidgetClipping::ClipToBounds);
    }

    TargetFrameCoreUX::StyleBorder(PanelBorder, TargetFrameCoreUX::PanelColor, FMargin(20.0f), 10.0f);
    TargetFrameCoreUX::StyleBorder(OnboardingBorder, TargetFrameCoreUX::PanelColor, FMargin(24.0f), 10.0f);

    TargetFrameCoreUX::StyleBorder(HeaderCard, TargetFrameCoreUX::CardElevatedColor, FMargin(16.0f), 9.0f);
    TargetFrameCoreUX::StyleBorder(HardwarePillBorder, TargetFrameCoreUX::AccentColor.CopyWithNewOpacity(0.18f), FMargin(14.0f, 8.0f), 18.0f);
    TargetFrameCoreUX::StyleBorder(ControlsCard, TargetFrameCoreUX::CardColor, FMargin(14.0f), 8.0f);
    TargetFrameCoreUX::StyleBorder(OverviewCard, TargetFrameCoreUX::CardColor, FMargin(14.0f), 8.0f);
    TargetFrameCoreUX::StyleBorder(RenderingCard, TargetFrameCoreUX::CardColor, FMargin(14.0f), 8.0f);
    TargetFrameCoreUX::StyleBorder(HardwareCard, TargetFrameCoreUX::CardColor, FMargin(14.0f), 8.0f);
    TargetFrameCoreUX::StyleBorder(ActionCard, TargetFrameCoreUX::CardColor, FMargin(14.0f), 8.0f);
    TargetFrameCoreUX::StyleBorder(ShippingCapsuleCard, TargetFrameCoreUX::CardColor, FMargin(14.0f), 8.0f);
    TargetFrameCoreUX::StyleBorder(NotesCard, TargetFrameCoreUX::CardColor, FMargin(14.0f), 8.0f);
    TargetFrameCoreUX::StyleBorder(EventsCard, TargetFrameCoreUX::CardColor, FMargin(14.0f), 8.0f);
    TargetFrameCoreUX::StyleBorder(OnboardingHardwareCard, TargetFrameCoreUX::CardColor, FMargin(14.0f), 8.0f);
    TargetFrameCoreUX::StyleBorder(OnboardingRecommendationCard, TargetFrameCoreUX::CardColor, FMargin(14.0f), 8.0f);

    TargetFrameCoreUX::StyleButton(LauncherButton, TargetFrameCoreUX::BrandColor.CopyWithNewOpacity(0.88f), TargetFrameCoreUX::BrandColor, TargetFrameCoreUX::AccentColor);
    TargetFrameCoreUX::StyleButton(Preset30Button, TargetFrameCoreUX::CardElevatedColor, TargetFrameCoreUX::AccentColor.CopyWithNewOpacity(0.88f), TargetFrameCoreUX::AccentColor);
    TargetFrameCoreUX::StyleButton(Preset45Button, TargetFrameCoreUX::CardElevatedColor, TargetFrameCoreUX::AccentColor.CopyWithNewOpacity(0.88f), TargetFrameCoreUX::AccentColor);
    TargetFrameCoreUX::StyleButton(Preset60Button, TargetFrameCoreUX::AccentColor.CopyWithNewOpacity(0.90f), TargetFrameCoreUX::AccentColor, TargetFrameCoreUX::BrandColor);
    TargetFrameCoreUX::StyleButton(Preset90Button, TargetFrameCoreUX::CardElevatedColor, TargetFrameCoreUX::BrandColor.CopyWithNewOpacity(0.92f), TargetFrameCoreUX::BrandColor);
    TargetFrameCoreUX::StyleButton(ApplyPolicyButton, TargetFrameCoreUX::BrandColor.CopyWithNewOpacity(0.86f), TargetFrameCoreUX::BrandColor, TargetFrameCoreUX::AccentColor);
    TargetFrameCoreUX::StyleButton(RebenchmarkButton, TargetFrameCoreUX::CardElevatedColor, TargetFrameCoreUX::AccentColor.CopyWithNewOpacity(0.80f), TargetFrameCoreUX::AccentColor);
    TargetFrameCoreUX::StyleButton(ClosePanelButton, TargetFrameCoreUX::CardElevatedColor, TargetFrameCoreUX::WarningColor.CopyWithNewOpacity(0.90f), TargetFrameCoreUX::WarningColor);
    TargetFrameCoreUX::StyleButton(RunGuidedSetupButton, TargetFrameCoreUX::AccentColor, TargetFrameCoreUX::AccentColor.CopyWithNewOpacity(0.92f), TargetFrameCoreUX::BrandColor);
    TargetFrameCoreUX::StyleButton(OpenControlPanelButton, TargetFrameCoreUX::BrandColor.CopyWithNewOpacity(0.88f), TargetFrameCoreUX::BrandColor, TargetFrameCoreUX::AccentColor);
    TargetFrameCoreUX::StyleButton(SkipOnboardingButton, TargetFrameCoreUX::CardElevatedColor, TargetFrameCoreUX::WarningColor.CopyWithNewOpacity(0.90f), TargetFrameCoreUX::WarningColor);

    TargetFrameCoreUX::StyleText(HeaderTitleText, TargetFrameCoreUX::TextColor, 19, TEXT("Bold"), 620.0f);
    TargetFrameCoreUX::StyleText(HeaderSummaryText, TargetFrameCoreUX::MutedTextColor, 12, TEXT("Regular"), 620.0f);
    TargetFrameCoreUX::StyleText(HeaderHardwarePillText, TargetFrameCoreUX::AccentColor, 11, TEXT("Bold"), 180.0f, ETextJustify::Center);
    TargetFrameCoreUX::StyleText(LauncherLabelText, TargetFrameCoreUX::PanelColor, 12, TEXT("Bold"), 0.0f, ETextJustify::Center);
    TargetFrameCoreUX::StyleText(OverviewValueText, TargetFrameCoreUX::TextColor, 12, TEXT("Regular"), 560.0f);
    TargetFrameCoreUX::StyleText(RenderingValueText, TargetFrameCoreUX::TextColor, 12, TEXT("Regular"), 560.0f);
    TargetFrameCoreUX::StyleText(HardwareValueText, TargetFrameCoreUX::TextColor, 12, TEXT("Regular"), 560.0f);
    TargetFrameCoreUX::StyleText(ActionValueText, TargetFrameCoreUX::TextColor, 12, TEXT("Regular"), 560.0f);
    TargetFrameCoreUX::StyleText(ShippingCapsuleValueText, TargetFrameCoreUX::TextColor, 12, TEXT("Regular"), 560.0f);
    TargetFrameCoreUX::StyleText(OnboardingTitleText, TargetFrameCoreUX::TextColor, 20, TEXT("Bold"), 620.0f);
    TargetFrameCoreUX::StyleText(OnboardingBodyText, TargetFrameCoreUX::MutedTextColor, 12, TEXT("Regular"), 620.0f);
    TargetFrameCoreUX::StyleText(OnboardingHardwareText, TargetFrameCoreUX::TextColor, 12, TEXT("Regular"), 600.0f);
    TargetFrameCoreUX::StyleText(OnboardingRecommendationText, TargetFrameCoreUX::TextColor, 12, TEXT("Regular"), 600.0f);

    // Normalize any designer-authored labels and button captions to a valid engine font.
    TargetFrameCoreUX::NormalizeWidgetTreeFonts(WidgetTree);
}

void UTargetFrameCoreUserExperienceWidget::RefreshPresentation()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UTargetFrameCoreSubsystem* Subsystem = GameInstance->GetSubsystem<UTargetFrameCoreSubsystem>())
        {
            const FTargetFrameCoreStatus Status = Subsystem->GetStatus();
            const FTargetFrameCoreSetupSummary SetupSummary = Subsystem->GetSetupSummary();

            if (HeaderTitleText != nullptr)
            {
                HeaderTitleText->SetText(FText::FromString(FString::Printf(TEXT("%s profile ready for %d FPS"), *SetupSummary.ProfileName, Status.TargetFPS)));
            }

            if (HeaderSummaryText != nullptr)
            {
                HeaderSummaryText->SetText(FText::FromString(TargetFrameCoreUX::CompactText(SetupSummary.Summary, 96)));
            }

            if (HeaderHardwarePillText != nullptr)
            {
                HeaderHardwarePillText->SetText(FText::FromString(TargetFrameCoreUX::MakeHardwarePillLabel(Status)));
            }

            if (LauncherLabelText != nullptr)
            {
                LauncherLabelText->SetText(FText::FromString(TEXT("Open TargetFrameCore")));
            }

            if (OverviewValueText != nullptr)
            {
                OverviewValueText->SetText(FText::FromString(FString::Printf(
                    TEXT("Live %.1f FPS against a %d FPS target.\nQuality %d, render scale %.1f%%.\nDynamic resolution %s."),
                    Status.SmoothedFPS,
                    Status.TargetFPS,
                    Status.OverallQualityLevel,
                    Status.ResolutionScale,
                    Status.bDynamicResolutionManaged ? TEXT("enabled") : TEXT("idle"))));
                OverviewValueText->SetColorAndOpacity(FSlateColor(
                    Status.SmoothedFPS >= static_cast<float>(Status.TargetFPS) ? TargetFrameCoreUX::PositiveColor : TargetFrameCoreUX::WarningColor));
            }

            if (RenderingValueText != nullptr)
            {
                RenderingValueText->SetText(FText::FromString(FString::Printf(
                    TEXT("Secondary UI %.0f%%"), Status.SecondaryScreenPercentage))); }

            if (HardwareValueText != nullptr)
            {
                HardwareValueText->SetText(FText::FromString(FString::Printf(
                    TEXT("%s\n%d logical cores, %d GB RAM\nTexture pool %d MB"),
                    *TargetFrameCoreUX::CompactText(Status.Hardware.GPUBrand, 48),
                    Status.Hardware.LogicalCoreCount,
                    Status.Hardware.SystemMemoryGB,
                    Status.TexturePoolBudgetMB)));
            }

            if (ActionValueText != nullptr)
            {
                ActionValueText->SetText(FText::FromString(FString::Printf(
                    TEXT("%s\n%s"),
                    Status.LastPolicyAction.IsEmpty() ? TEXT("Monitoring frame budget") : *TargetFrameCoreUX::CompactText(Status.LastPolicyAction, 72),
                    Status.LastPolicyReason.IsEmpty() ? TEXT("Waiting for the next TargetFrameCore decision.") : *TargetFrameCoreUX::CompactText(Status.LastPolicyReason, 88))));
            }

            if (ShippingCapsuleValueText != nullptr)
            {
                const FTargetFrameCoreProjectCapabilities& Capabilities = SetupSummary.ProjectCapabilities;
                const FString CapsuleStatus = Status.bFireAndForgetShippingEnabled ? TEXT("Active") : TEXT("Off");
                const FString CapsuleLock = !Status.bFireAndForgetShippingEnabled
                    ? TEXT("Inactive")
                    : (Status.bPolicyLockEngaged ? TEXT("Locked after stabilization") : TEXT("Stabilizing"));
                const FString CapsuleMode = TargetFrameCoreUX::DescribeShippingMode(Capabilities);
                const FString CapsuleControls = TargetFrameCoreUX::BuildShippingControlLabel(Capabilities);
                const int32 SupportedControls = TargetFrameCoreUX::CountSupportedShippingControls(Capabilities);

                ShippingCapsuleValueText->SetText(FText::FromString(FString::Printf(
                    TEXT("Status: %s | Lock: %s\nMode: %s (%d/7 controls)\nTarget: %d FPS for %s tier\nControls: %s"),
                    *CapsuleStatus,
                    *CapsuleLock,
                    *CapsuleMode,
                    SupportedControls,
                    Status.TargetFPS,
                    *SetupSummary.ProfileName,
                    *CapsuleControls)));

                const FLinearColor CapsuleColor = !Status.bFireAndForgetShippingEnabled
                    ? TargetFrameCoreUX::MutedTextColor
                    : (Status.bPolicyLockEngaged ? TargetFrameCoreUX::PositiveColor : TargetFrameCoreUX::BrandColor);
                ShippingCapsuleValueText->SetColorAndOpacity(FSlateColor(CapsuleColor));
            }

            RebuildTextList(SetupNotesList, SetupSummary.Notes, 5, TargetFrameCoreUX::TextColor);
            RebuildTextList(RecentEventsList, Status.RecentEvents, 6, TargetFrameCoreUX::MutedTextColor);

            if (OnboardingTitleText != nullptr)
            {
                OnboardingTitleText->SetText(FText::FromString(TEXT("Welcome to TargetFrameCore")));
            }

            if (OnboardingBodyText != nullptr)
            {
                OnboardingBodyText->SetText(FText::FromString(
                    TEXT("TargetFrameCore benchmarks the machine, picks a safe rendering posture, and keeps UE5 features inside a frame budget without asking players to hand-tune dozens of settings.")));
            }

            if (OnboardingHardwareText != nullptr)
            {
                OnboardingHardwareText->SetText(FText::FromString(FString::Printf(
                    TEXT("Tier: %s\nGPU: %s\nScores: GPU %.1f | CPU %.1f\nGraphics memory: %s"),
                    *SetupSummary.ProfileName,
                    *TargetFrameCoreUX::CompactText(Status.Hardware.GPUBrand, 40),
                    Status.LastGPUBenchmarkScore,
                    Status.LastCPUBenchmarkScore,
                    *TargetFrameCoreUX::FormatVRAMLabel(Status.Hardware.DeviceWorkingMemoryMB))));
            }

            if (OnboardingRecommendationText != nullptr)
            {
                OnboardingRecommendationText->SetText(FText::FromString(FString::Printf(
TEXT("Target %d FPS with balanced safeguards.\nSecondary UI: %.0f%%\nLow-VRAM mode: %s\nShipping capsule: %s"),                    Status.TargetFPS,
                    Status.SecondaryScreenPercentage,
                    Status.bLowVRAMMode ? TEXT("On") : TEXT("Off"),
                    Status.bFireAndForgetShippingEnabled ? TEXT("Active") : TEXT("Off"))));
            }
        }
    }
}

void UTargetFrameCoreUserExperienceWidget::ApplyInputMode() const
{
    if (!CachedPlayerController.IsValid())
    {
        return;
    }

    const bool bInteractive = bControlPanelVisible || bOnboardingVisible;
    CachedPlayerController->bShowMouseCursor = bInteractive;
    CachedPlayerController->bEnableClickEvents = bInteractive;
    CachedPlayerController->bEnableMouseOverEvents = bInteractive;

    if (bInteractive)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetHideCursorDuringCapture(false);
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        CachedPlayerController->SetInputMode(InputMode);
    }
    else
    {
        FInputModeGameOnly InputMode;
        CachedPlayerController->SetInputMode(InputMode);
    }
}

void UTargetFrameCoreUserExperienceWidget::UpdateShowcaseFocus() const
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    const bool bShouldFocusPresentation = bControlPanelVisible || bOnboardingVisible;
    for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
    {
        AActor* ShowcaseActor = *ActorIt;
        if (ShowcaseActor == nullptr)
        {
            continue;
        }

        if (UFunction* FocusFunction = ShowcaseActor->FindFunction(TEXT("SetPresentationFocus")))
        {
            struct FShowcaseFocusParams
            {
                bool bFocused;
            };

            FShowcaseFocusParams Params{ bShouldFocusPresentation };
            ShowcaseActor->ProcessEvent(FocusFunction, &Params);
            UE_LOG(
                LogTargetFrameCore,
                Verbose,
                TEXT("TargetFrameCore UX presentation focus -> %s on actor %s"),
                bShouldFocusPresentation ? TEXT("enabled") : TEXT("disabled"),
                *ShowcaseActor->GetName());
            break;
        }
    }
}

void UTargetFrameCoreUserExperienceWidget::SetControlPanelVisible(bool bVisible)
{
    bControlPanelVisible = bVisible;

    if (PanelBorder != nullptr)
    {
        PanelBorder->SetVisibility(bControlPanelVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (LauncherButton != nullptr)
    {
        LauncherButton->SetVisibility((bControlPanelVisible || bOnboardingVisible) ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }

    if (ScrimBorder != nullptr)
    {
        ScrimBorder->SetVisibility((bControlPanelVisible || bOnboardingVisible) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    UpdateShowcaseFocus();

    UE_LOG(LogTargetFrameCore, Verbose, TEXT("TargetFrameCore UX control panel visibility changed: %s"), bControlPanelVisible ? TEXT("Visible") : TEXT("Hidden"));
}

void UTargetFrameCoreUserExperienceWidget::SetOnboardingVisible(bool bVisible)
{
    bOnboardingVisible = bVisible;

    if (OnboardingBorder != nullptr)
    {
        OnboardingBorder->SetVisibility(bOnboardingVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (LauncherButton != nullptr)
    {
        LauncherButton->SetVisibility((bControlPanelVisible || bOnboardingVisible) ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }

    if (ScrimBorder != nullptr)
    {
        ScrimBorder->SetVisibility((bControlPanelVisible || bOnboardingVisible) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    UpdateShowcaseFocus();

    UE_LOG(LogTargetFrameCore, Verbose, TEXT("TargetFrameCore UX onboarding visibility changed: %s"), bOnboardingVisible ? TEXT("Visible") : TEXT("Hidden"));
}

void UTargetFrameCoreUserExperienceWidget::SetTargetFPSAndApply(int32 NewTargetFPS)
{
    UTargetFrameCoreBlueprintLibrary::SetTargetFrameCoreTargetFPS(this, NewTargetFPS);
    RefreshPresentation();
}

void UTargetFrameCoreUserExperienceWidget::RebuildTextList(UVerticalBox* ListWidget, const TArray<FString>& Entries, int32 MaxEntries, const FLinearColor& EntryColor)
{
    if (ListWidget == nullptr || WidgetTree == nullptr)
    {
        return;
    }

    ListWidget->ClearChildren();

    if (Entries.Num() == 0)
    {
        AddListEntry(ListWidget, TEXT("No entries yet."), TargetFrameCoreUX::MutedTextColor);
        return;
    }

    const int32 ClampedCount = FMath::Min(Entries.Num(), MaxEntries);
    for (int32 Index = 0; Index < ClampedCount; ++Index)
    {
        AddListEntry(ListWidget, Entries[Index], EntryColor);
    }
}

UTextBlock* UTargetFrameCoreUserExperienceWidget::AddListEntry(UVerticalBox* ListWidget, const FString& Text, const FLinearColor& EntryColor)
{
    if (ListWidget == nullptr || WidgetTree == nullptr)
    {
        return nullptr;
    }

    UTextBlock* EntryText = TargetFrameCoreUX::CreateText(WidgetTree, FString::Printf(TEXT("- %s"), *TargetFrameCoreUX::CompactText(Text, 88)), EntryColor, 11);
    EntryText->SetAutoWrapText(true);
    EntryText->SetWrapTextAt(520.0f);

    UVerticalBoxSlot* EntrySlot = ListWidget->AddChildToVerticalBox(EntryText);
    EntrySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    return EntryText;
}

void UTargetFrameCoreUserExperienceWidget::HandleOpenLauncher()
{
    ShowControlPanel();
}

void UTargetFrameCoreUserExperienceWidget::HandleClosePanel()
{
    HideControlPanel();
}

void UTargetFrameCoreUserExperienceWidget::HandleApplyPolicy()
{
    UTargetFrameCoreBlueprintLibrary::ApplyTargetFrameCorePolicy(this);
    RefreshPresentation();
}

void UTargetFrameCoreUserExperienceWidget::HandleRebenchmark()
{
    UTargetFrameCoreBlueprintLibrary::StartTargetFrameCore(this, true);
    RefreshPresentation();
}

void UTargetFrameCoreUserExperienceWidget::HandlePreset30()
{
    SetTargetFPSAndApply(30);
}

void UTargetFrameCoreUserExperienceWidget::HandlePreset45()
{
    SetTargetFPSAndApply(45);
}

void UTargetFrameCoreUserExperienceWidget::HandlePreset60()
{
    SetTargetFPSAndApply(60);
}

void UTargetFrameCoreUserExperienceWidget::HandlePreset90()
{
    SetTargetFPSAndApply(90);
}

void UTargetFrameCoreUserExperienceWidget::HandleRunGuidedSetup()
{
    UTargetFrameCoreBlueprintLibrary::StartTargetFrameCore(this, true);
    if (bRememberOnboarding)
    {
        UTargetFrameCoreBlueprintLibrary::SetTargetFrameCoreOnboardingCompleted(true);
    }

    SetOnboardingVisible(false);
    SetControlPanelVisible(true);
    RefreshPresentation();
    ApplyInputMode();
}

void UTargetFrameCoreUserExperienceWidget::HandleOpenControlPanelFromOnboarding()
{
    if (bRememberOnboarding)
    {
        UTargetFrameCoreBlueprintLibrary::SetTargetFrameCoreOnboardingCompleted(true);
    }

    SetOnboardingVisible(false);
    SetControlPanelVisible(true);
    RefreshPresentation();
    ApplyInputMode();
}

void UTargetFrameCoreUserExperienceWidget::HandleSkipOnboarding()
{
    if (bRememberOnboarding)
    {
        UTargetFrameCoreBlueprintLibrary::SetTargetFrameCoreOnboardingCompleted(true);
    }

    SetOnboardingVisible(false);
    SetControlPanelVisible(true);
    RefreshPresentation();
    ApplyInputMode();
}

void UTargetFrameCoreUserExperienceWidget::HandleRememberOnboardingChanged(bool bIsChecked)
{
    bRememberOnboarding = bIsChecked;
}
