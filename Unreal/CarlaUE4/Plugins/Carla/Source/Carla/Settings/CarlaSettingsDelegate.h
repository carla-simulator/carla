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
  /** */
  UCarlaSettingsDelegate();

  /** */
  void RegisterSpawnHandler(UWorld *World);

  /** */
  void OnActorSpawned(AActor *Actor);

  /** */
  void ApplyQualitySettingsLevelPostRestart() const;

  /** */  
  void ApplyQualitySettingsLevelPreRestart() const{/** @TODO: implement */}

private: 
  /***/
  void LaunchLowQualityCommands(UWorld* world) const;
  /***/
  void SetAllLightsLowQuality(UWorld* world) const;
  /***/
  void SetAllRoadsLowQuality(UWorld* world) const;
  /***/
  void SetAllActorsDrawDistance(UWorld* world, const float max_draw_distance) const;
  /***/
  void SetPostProcessEffectsEnabled(UWorld* world, bool enabled) const;
  /***/
  void ApplyLowQualitySettings() const;

private:
  /** */
  UCarlaSettings* CarlaSettings = nullptr;

  /** */
  FOnActorSpawned::FDelegate ActorSpawnedDelegate;
};
