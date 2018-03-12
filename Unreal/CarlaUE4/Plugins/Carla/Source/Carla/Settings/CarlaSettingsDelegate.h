// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CarlaSettingsDelegate.generated.h"

class UCarlaSettings;

/// Used to set settings for every actor that is spawned into the world.
UCLASS()
class CARLA_API UCarlaSettingsDelegate : public UObject
{
  GENERATED_BODY()

public:
  /** Constructor */
  UCarlaSettingsDelegate();

  /** Create the event trigger handler for all new spawned actors to be processed with a custom function here */
  void RegisterSpawnHandler(UWorld *World);

  /** Function to apply to the actor that is being spawned to apply the current settings */
  void OnActorSpawned(AActor *Actor);

  /** After loading a level, apply the current settings */
  void ApplyQualitySettingsLevelPostRestart(UWorld* InWorld);

  /** Before loading a level, apply the current settings */  
  void ApplyQualitySettingsLevelPreRestart(UWorld* InWorld);

private: 
  /** Check that the world ,instance and settings are valid and save the CarlaSettings instance 
   * @param world used to get the instance of CarlaSettings 
   */
  void CheckCarlaSettings(UWorld* world);

  /** Execute engine commands to apply the low quality settings to the world */
  void LaunchLowQualityCommands(UWorld* world) const;

  /** */
  void SetAllLightsLowQuality(UWorld* world) const;

  /** */
  void SetAllRoadsLowQuality(UWorld* world) const;

  /** */
  void SetActorComponentsDrawDistance(AActor* actor, const float max_draw_distance) const;

  /** */
  void SetAllActorsDrawDistance(UWorld* world, const float max_draw_distance) const;

  /** */
  void SetPostProcessEffectsEnabled(UWorld* world, bool enabled) const;

  /** */
  void ApplyLowQualitySettings(UWorld* world) const;

private:
  /** */
  UCarlaSettings* CarlaSettings = nullptr;

  /** */
  FOnActorSpawned::FDelegate ActorSpawnedDelegate;
};
