// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "BlueprintLibary/PostProcessJsonUtils.h"

#include "Components/PostProcessComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

static TAutoConsoleVariable<FString> CVarCarlaPostProcessProfile(
    TEXT("carla.PostProcess.Profile"),
    TEXT("AutomotiveHDR"),
    TEXT("Camera profile (Content/Carla/Config/PostProcess/<name>.json) loaded by RGB sensors ")
    TEXT("spawned without post_process_profile (or with \"Default\")."),
    ECVF_Default);

static TAutoConsoleVariable<FString> CVarCarlaPostProcessViewportProfile(
    TEXT("carla.PostProcess.ViewportProfile"),
    TEXT("Cinematic"),
    TEXT("Camera profile the sky rig's post process loads at BeginPlay: the PIE viewport, ")
    TEXT("the spectator and the server window. Set it to the sensor profile to see what the ")
    TEXT("RGB sensors see."),
    ECVF_Default);

static FString ProfileOrFallback(const TAutoConsoleVariable<FString>& CVar, const TCHAR* Fallback)
{
    const FString Name = CVar.GetValueOnGameThread();
    return Name.IsEmpty() ? FString(Fallback) : Name;
}

FString UPostProcessJsonUtils::GetActiveProfileName()
{
    return ProfileOrFallback(CVarCarlaPostProcessProfile, TEXT("AutomotiveHDR"));
}

FString UPostProcessJsonUtils::GetViewportProfileName()
{
    return ProfileOrFallback(CVarCarlaPostProcessViewportProfile, TEXT("Cinematic"));
}

FString UPostProcessJsonUtils::ResolveProfileName(const FString& Requested)
{
    // There is no Default.json any more: "Default" in any case (scripts written
    // when it was a file) means the sensor default, like an empty name.
    if (Requested.IsEmpty() || Requested.Equals(TEXT("Default"), ESearchCase::IgnoreCase))
        return GetActiveProfileName();
    return Requested;
}

bool UPostProcessJsonUtils::SaveAllPostProcessToJson(APostProcessVolume* Volume, const FString& FileName)
{
    FPostProcessSettingsWrapper Wrapper;
    Wrapper.Settings = Volume->Settings;

    FString OutputString;
    if (FJsonObjectConverter::UStructToJsonObjectString(Wrapper, OutputString))
    {
        FString FullPath = GetPostProcessConfigPath(FileName);
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(FullPath), true);
        return FFileHelper::SaveStringToFile(OutputString, *FullPath);
    }

    return false;
}

bool UPostProcessJsonUtils::SaveAllPostProcessComponentToJson(UPostProcessComponent* Volume, const FString& FileName)
{
    FPostProcessSettingsWrapper Wrapper;
    Wrapper.Settings = Volume->Settings;

    FString OutputString;
    if (FJsonObjectConverter::UStructToJsonObjectString(Wrapper, OutputString))
    {
        FString FullPath = GetPostProcessConfigPath(FileName);
        // Ensure the directory exists before saving
        FString DirectoryPath = FPaths::GetPath(FullPath);
        if (!IFileManager::Get().DirectoryExists(*DirectoryPath))
        {
            IFileManager::Get().MakeDirectory(*DirectoryPath, true);
        }
        return FFileHelper::SaveStringToFile(OutputString, *FullPath);
    }

    return false;
}

bool UPostProcessJsonUtils::LoadAllPostProcessFromJsonToPostProcessComponent(UPostProcessComponent* Volume, const FString& FileName)
{
    FString FullPath = GetPostProcessConfigPath(FileName);
    FString InputString;

    if (FFileHelper::LoadFileToString(InputString, *FullPath))
    {
        FPostProcessSettingsWrapper Wrapper;
        if (FJsonObjectConverter::JsonObjectStringToUStruct(InputString, &Wrapper, 0, 0))
        {
            Volume->Settings = Wrapper.Settings; 
            return true;
        }
    }
    return false;
}

bool UPostProcessJsonUtils::LoadAllPostProcessFromJsonToPostProcessVolume(APostProcessVolume* Volume, const FString& FileName)
{
    FString FullPath = GetPostProcessConfigPath(FileName);
    FString InputString;

    if (FFileHelper::LoadFileToString(InputString, *FullPath))
    {
        FPostProcessSettingsWrapper Wrapper;
        if (FJsonObjectConverter::JsonObjectStringToUStruct(InputString, &Wrapper, 0, 0))
        {
            Volume->Settings = Wrapper.Settings; 
            return true;
        }
    }
    return false;
}


bool UPostProcessJsonUtils::LoadAllPostProcessFromJsonToSceneCapture(USceneCaptureComponent2D* SensorCamera, const FString& FileName)
{
    FString FullPath = GetPostProcessConfigPath(FileName);
    FString InputString;

    if (FFileHelper::LoadFileToString(InputString, *FullPath))
    {
        FPostProcessSettingsWrapper Wrapper;
        if (FJsonObjectConverter::JsonObjectStringToUStruct(InputString, &Wrapper, 0, 0))
        {
            SensorCamera->PostProcessSettings = Wrapper.Settings;
            return true;
        }
    }
    return false;
}

TArray<FString> UPostProcessJsonUtils::GetAvailablePostProcessProfileNames()
{
    TArray<FString> ProfileNames;
    const FString ConfigDir = FPaths::ProjectContentDir() / TEXT("Carla/Config/PostProcess/");
    IFileManager::Get().FindFiles(ProfileNames, *(ConfigDir / TEXT("*.json")), true, false);
    for (FString& Name : ProfileNames)
    {
        Name = FPaths::GetBaseFilename(Name);
    }
    ProfileNames.Sort();
    return ProfileNames;
}
