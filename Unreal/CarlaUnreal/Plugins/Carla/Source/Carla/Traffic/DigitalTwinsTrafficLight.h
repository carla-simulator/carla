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
 * Its lamps are the Traffic Light Tool rig meshes UMapLogicParser re-parents onto it;
 * ATrafficLightBase::UpdateToolRigLamps lights them by state.
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
};
