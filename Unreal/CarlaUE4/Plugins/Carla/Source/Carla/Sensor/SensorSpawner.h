// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawner.h"
#include "Carla/Actor/ActorSpawnResult.h"

#include "GameFramework/Actor.h"

#include "SensorSpawner.generated.h"

/// Object in charge of spawning sensors.
UCLASS()
class CARLA_API ASensorSpawner : public AActorSpawner
{
  GENERATED_BODY()

  TArray<FActorDefinition> MakeDefinitions() final;

  FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) final;
};
