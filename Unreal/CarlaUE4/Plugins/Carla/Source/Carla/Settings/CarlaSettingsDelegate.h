// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "CarlaSettingsDelegate.generated.h"

/// Used to set settings for every actor that is spawned into the world.
UCLASS(BlueprintType)
class CARLA_API UCarlaSettingsDelegate : public UObject
{
  GENERATED_BODY()

public:
  /** Constructor */
  UCarlaSettingsDelegate();

  /** Reset settings to default */
  void Reset();

  /** Create the event trigger handler for all new spawned actors to be processed with a custom function here */
  void RegisterSpawnHandler(UWorld *World);

  /** After loading a level, apply the current settings */
  UFUNCTION(BlueprintCallable, Category="CARLA Settings", meta=(HidePin="InWorld"))
  void ApplyQualitySettingsLevelPostRestart();

  /** Before loading a level, apply the current settings */  
  UFUNCTION(BlueprintCallable, Category="CARLA Settings", meta=(HidePin="InWorld"))
  void ApplyQualitySettingsLevelPreRestart();

private:
  UWorld* GetLocalWorld();
  /** Function to apply to the actor that is being spawned to apply the current settings */
  void OnActorSpawned(AActor *Actor);
  /** Check that the world ,instance and settings are valid and save the CarlaSettings instance 
   * @param world used to get the instance of CarlaSettings 
   */
  void CheckCarlaSettings(UWorld* world);

  /** Execute engine commands to apply the low quality settings to the world */
  void LaunchLowQualityCommands(UWorld* world) const;

  /** */
  void SetAllRoads(UWorld* world,const float max_draw_distance, const TArray<FStaticMaterial> &road_pieces_materials) const;

  /** */
  void SetActorComponentsDrawDistance(AActor* actor, const float max_draw_distance) const;

  /** */
  void SetAllActorsDrawDistance(UWorld* world, const float max_draw_distance) const;

  /** */
  void SetPostProcessEffectsEnabled(UWorld* world, const bool enabled) const;
  
  /** Execute engine commands to apply the epic quality settings to the world */
  void LaunchEpicQualityCommands(UWorld* world) const;

  /** */
  void SetAllLights(UWorld* world, const float max_distance_fade, const bool cast_shadows, const bool hide_non_directional) const;

private:

  /** currently applied settings level after level is restarted */
  static EQualitySettingsLevel AppliedLowPostResetQualitySettingsLevel;

  /** */
  UCarlaSettings* CarlaSettings = nullptr;

  /** */
  FOnActorSpawned::FDelegate ActorSpawnedDelegate;
};
