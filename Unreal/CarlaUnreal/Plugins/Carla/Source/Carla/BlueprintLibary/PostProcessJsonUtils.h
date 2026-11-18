// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/Paths.h"
#include "PostProcessJsonUtils.generated.h"

USTRUCT(BlueprintType)
struct FPostProcessSettingsWrapper
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPostProcessSettings Settings;
};


class UPostProcessComponent;
class USceneCaptureComponent2D;

UCLASS()
class CARLA_API UPostProcessJsonUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "PostProcess|JSON")
    static bool SaveAllPostProcessToJson(APostProcessVolume* Volume, const FString& FileName);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|JSON")
    static bool LoadAllPostProcessFromJsonToPostProcessVolume(APostProcessVolume* Volume, const FString& FileName);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|JSON")
    static bool SaveAllPostProcessComponentToJson(UPostProcessComponent * Volume, const FString& FileName);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|JSON")
    static bool LoadAllPostProcessFromJsonToPostProcessComponent(UPostProcessComponent * Volume, const FString& FileName);


    UFUNCTION(BlueprintCallable, Category = "PostProcess|JSON")
    static bool LoadAllPostProcessFromJsonToSceneCapture(USceneCaptureComponent2D* SensorCamera, const FString& FileName);

    // Names (without ".json") of every profile currently saved under
    // Content/Carla/Config/PostProcess/. Backs the "Get Options" dropdown on
    // APostProcessProfileVolume::ProfileName so artists can pick an existing
    // profile or type a new one to create it on the next SaveProfile.
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PostProcess|JSON")
    static TArray<FString> GetAvailablePostProcessProfileNames();

    // The camera profile RGB sensors load when spawned without
    // post_process_profile (or with the legacy "Default"). Set with
    // carla.PostProcess.Profile.
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PostProcess|JSON")
    static FString GetActiveProfileName();

    // The camera profile the sky rig's post process (PIE viewport, spectator,
    // server window) loads at BeginPlay. Set with carla.PostProcess.ViewportProfile.
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PostProcess|JSON")
    static FString GetViewportProfileName();

    // Profile name given to a sensor -> file to load. Empty or "Default" (any
    // case) means the sensor default, GetActiveProfileName().
    static FString ResolveProfileName(const FString& Requested);

    static FString GetPostProcessConfigPath(const FString& FileName)
    {
        return FPaths::ProjectContentDir() / TEXT("Carla/Config/PostProcess/") + FileName + TEXT(".json");
    }
};
