// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawnResult.h"
#include "Carla/Actor/CarlaActorFactory.h"

#include "TriggerFactory.generated.h"

/// Factory in charge of spawning sensors. This factory is able to spawn every
/// sensor registered in carla::sensor::SensorRegistry.
UCLASS()
class CARLA_API ATriggerFactory : public ACarlaActorFactory
{
  GENERATED_BODY()

  /// Retrieve the definitions of all the sensors registered in the
  /// SensorRegistry. Sensors must implement a GetSensorDefinition() static
  /// method.
  TArray<FActorDefinition> GetDefinitions() final;

  FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) final;

};
