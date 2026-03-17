#include "TargetFrameCoreBlueprintLibrary.h"

#include "Misc/ConfigCacheIni.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TargetFrameCoreSubsystem.h"

namespace
{
    constexpr TCHAR TargetFrameCoreUserSettingsSection[] = TEXT("TargetFrameCore.UserExperience");
    constexpr TCHAR TargetFrameCoreOnboardingCompletedKey[] = TEXT("bOnboardingCompleted");
}

void UTargetFrameCoreBlueprintLibrary::StartTargetFrameCore(UObject* WorldContextObject, bool bForceRebenchmark)
{
    if (UTargetFrameCoreSubsystem* Subsystem = ResolveSubsystem(WorldContextObject))
    {
        Subsystem->StartAutomaticSetup(bForceRebenchmark);
    }
}

void UTargetFrameCoreBlueprintLibrary::SetTargetFrameCoreTargetFPS(UObject* WorldContextObject, int32 NewTargetFPS)
{
    if (UTargetFrameCoreSubsystem* Subsystem = ResolveSubsystem(WorldContextObject))
    {
        Subsystem->SetTargetFPS(NewTargetFPS);
        Subsystem->ApplyCurrentPolicy();
    }
}

void UTargetFrameCoreBlueprintLibrary::ApplyTargetFrameCorePolicy(UObject* WorldContextObject)
{
    if (UTargetFrameCoreSubsystem* Subsystem = ResolveSubsystem(WorldContextObject))
    {
        Subsystem->ApplyCurrentPolicy();
    }
}

void UTargetFrameCoreBlueprintLibrary::StartTargetFrameCoreFireAndForget(UObject* WorldContextObject, bool bForceRebenchmark)
{
    if (UTargetFrameCoreSubsystem* Subsystem = ResolveSubsystem(WorldContextObject))
    {
        Subsystem->SetFireAndForgetShippingEnabled(true);
        Subsystem->StartAutomaticSetup(bForceRebenchmark);
    }
}

FTargetFrameCoreStatus UTargetFrameCoreBlueprintLibrary::GetTargetFrameCoreStatus(const UObject* WorldContextObject)
{
    if (const UTargetFrameCoreSubsystem* Subsystem = ResolveSubsystem(WorldContextObject))
    {
        return Subsystem->GetStatus();
    }

    return FTargetFrameCoreStatus();
}

FTargetFrameCoreSetupSummary UTargetFrameCoreBlueprintLibrary::GetTargetFrameCoreSetupSummary(const UObject* WorldContextObject)
{
    if (const UTargetFrameCoreSubsystem* Subsystem = ResolveSubsystem(WorldContextObject))
    {
        return Subsystem->GetSetupSummary();
    }

    return FTargetFrameCoreSetupSummary();
}

FTargetFrameCoreProjectCapabilities UTargetFrameCoreBlueprintLibrary::GetTargetFrameCoreProjectCapabilities(const UObject* WorldContextObject)
{
    if (const UTargetFrameCoreSubsystem* Subsystem = ResolveSubsystem(WorldContextObject))
    {
        return Subsystem->GetProjectCapabilities();
    }

    return FTargetFrameCoreProjectCapabilities();
}

bool UTargetFrameCoreBlueprintLibrary::HasCompletedTargetFrameCoreOnboarding()
{
    bool bCompleted = false;
    if (GConfig != nullptr)
    {
        GConfig->GetBool(TargetFrameCoreUserSettingsSection, TargetFrameCoreOnboardingCompletedKey, bCompleted, GGameUserSettingsIni);
    }

    return bCompleted;
}

void UTargetFrameCoreBlueprintLibrary::SetTargetFrameCoreOnboardingCompleted(bool bCompleted)
{
    if (GConfig == nullptr)
    {
        return;
    }

    GConfig->SetBool(TargetFrameCoreUserSettingsSection, TargetFrameCoreOnboardingCompletedKey, bCompleted, GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);
}

UTargetFrameCoreSubsystem* UTargetFrameCoreBlueprintLibrary::ResolveSubsystem(const UObject* WorldContextObject)
{
    if (WorldContextObject == nullptr)
    {
        return nullptr;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (World == nullptr)
    {
        return nullptr;
    }

    UGameInstance* GameInstance = World->GetGameInstance();
    if (GameInstance == nullptr)
    {
        return nullptr;
    }

    return GameInstance->GetSubsystem<UTargetFrameCoreSubsystem>();
}
