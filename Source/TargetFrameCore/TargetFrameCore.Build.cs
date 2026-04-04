// Copyright (c) 2026 GregOrigin. All Rights Reserved.
using UnrealBuildTool;

public class TargetFrameCore : ModuleRules
{
    public TargetFrameCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "DeveloperSettings",
                "Engine",
                "UMG"
            });

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "Projects",
                "RHI",
                "Slate",
                "SlateCore"
            });
    }
}
