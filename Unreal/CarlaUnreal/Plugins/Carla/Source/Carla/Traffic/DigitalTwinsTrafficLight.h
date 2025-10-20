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

  /// Get the emissive color for a given traffic light state
  FLinearColor GetColorForState(ETrafficLightState State) const;

  // Maps each material to its light type (Red=0, Yellow=1, Green=2)
  // Built during BeginPlay by analyzing initial emissive colors
  TMap<UMaterialInstanceDynamic*, ETrafficLightState> MaterialToLightType;
};
