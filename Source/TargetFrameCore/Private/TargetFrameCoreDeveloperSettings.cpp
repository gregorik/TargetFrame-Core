// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#include "TargetFrameCoreDeveloperSettings.h"

#define LOCTEXT_NAMESPACE "TargetFrameCoreDeveloperSettings"

UTargetFrameCoreDeveloperSettings::UTargetFrameCoreDeveloperSettings()
{
}

FName UTargetFrameCoreDeveloperSettings::GetCategoryName() const
{
    return TEXT("Plugins");
}

FText UTargetFrameCoreDeveloperSettings::GetSectionText() const
{
    return LOCTEXT("SectionText", "TargetFrameCore");
}

FText UTargetFrameCoreDeveloperSettings::GetSectionDescription() const
{
    return LOCTEXT(
        "SectionDescription",
        "Runtime scalability policy for UE5 projects that need vendor-aware fallbacks, Nanite budgeting, crisp UI under upscaling, telemetry, and an optional fire-and-forget shipping capsule.");
}

#undef LOCTEXT_NAMESPACE
