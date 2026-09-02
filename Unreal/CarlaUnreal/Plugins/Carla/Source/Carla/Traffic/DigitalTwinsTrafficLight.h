// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Traffic/TrafficLightBase.h"
#include "DigitalTwinsTrafficLight.generated.h"

/**
 * Traffic light class specifically for DigitalTwins meshes.
 * Implements visual material updates for DigitalTwins atlas system.
 */
UCLASS()
class CARLA_API ADigitalTwinsTrafficLight : public ATrafficLightBase
{
  GENERATED_BODY()

public:
  ADigitalTwinsTrafficLight(const FObjectInitializer &ObjectInitializer);

  virtual void BeginPlay() override;

private:
  // Called when traffic light state changes via the LightChangeDispatcher
  UFUNCTION()
  void OnLightStateChanged();

private:
  /// Update material parameters for DigitalTwins meshes based on traffic light state
  void UpdateDigitalTwinsMaterials(ETrafficLightState State);

  /// Scan every "led_*" material slot of this actor's meshes and sort the lamps into states.
  /// Called lazily from UpdateDigitalTwinsMaterials, and again whenever the actor's mesh
  /// component count changes (UMapLogicParser re-parents the baked rig's meshes onto us).
  void BuildLampMap();

  /// Get the emissive color for a given traffic light state
  FLinearColor GetColorForState(ETrafficLightState State) const;

  /// Every lamp this actor drives, and the state it belongs to. A rig may carry any number of
  /// lamps per state (a mast arm repeats the three aspects on each head, a combi head adds
  /// arrow aspects), so this is not limited to three entries.
  TMap<UMaterialInstanceDynamic*, ETrafficLightState> MaterialToLightType;

  /// Lamps that belong to no state -- the stateless atlas glyphs (Tram, tram lines, Circle,
  /// Triangle) and anything unclassifiable. Baked lit at intensity 50000, so they are
  /// switched off once and never touched again.
  TArray<UMaterialInstanceDynamic*> DarkMaterials;

  /// Mesh component count the lamp map was built from.
  int32 CachedMeshComponentCount = 0;
};
