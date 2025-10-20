// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once


#include "BlueprintLibary/PostProcessJsonUtils.h"
#include "Components/PostProcessComponent.h"

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
