// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TargetFrameCoreTypes.h"
#include "TargetFrameCoreBlueprintLibrary.generated.h"

class UTargetFrameCoreSubsystem;
class UTargetFrameCoreUserExperienceWidget;

UCLASS()
class TARGETFRAMECORE_API UTargetFrameCoreBlueprintLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore", meta = (WorldContext = "WorldContextObject"))
    static void StartTargetFrameCore(UObject* WorldContextObject, bool bForceRebenchmark = false);

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore", meta = (WorldContext = "WorldContextObject"))
    static void SetTargetFrameCoreTargetFPS(UObject* WorldContextObject, int32 NewTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore", meta = (WorldContext = "WorldContextObject"))
    static void ApplyTargetFrameCorePolicy(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore", meta = (WorldContext = "WorldContextObject"))
    static void StartTargetFrameCoreFireAndForget(UObject* WorldContextObject, bool bForceRebenchmark = false);

    UFUNCTION(BlueprintPure, Category = "TargetFrameCore", meta = (WorldContext = "WorldContextObject"))
    static FTargetFrameCoreStatus GetTargetFrameCoreStatus(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category = "TargetFrameCore", meta = (WorldContext = "WorldContextObject"))
    static FTargetFrameCoreSetupSummary GetTargetFrameCoreSetupSummary(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category = "TargetFrameCore", meta = (WorldContext = "WorldContextObject"))
    static FTargetFrameCoreProjectCapabilities GetTargetFrameCoreProjectCapabilities(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category = "TargetFrameCore|UX")
    static bool HasCompletedTargetFrameCoreOnboarding();

    UFUNCTION(BlueprintCallable, Category = "TargetFrameCore|UX")
    static void SetTargetFrameCoreOnboardingCompleted(bool bCompleted = true);

private:
    static UTargetFrameCoreSubsystem* ResolveSubsystem(const UObject* WorldContextObject);
};
