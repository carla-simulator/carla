// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawner.h"
#include "Carla/Actor/ActorSpawnResult.h"

#include "GameFramework/Actor.h"

#include "ActorSpawnerBlueprint.generated.h"

/// Base class for Blueprints implementing AActorSpawner interface.
///
/// Blueprints deriving from this class are expected to override
/// GenerateDefinitions and SpawnActor functions.
UCLASS(Abstract, BlueprintType, Blueprintable)
class CARLA_API AActorSpawnerBlueprint : public AActorSpawner
{
  GENERATED_BODY()

public:

  TArray<FActorDefinition> MakeDefinitions() final
  {
    return GenerateDefinitions();
  }

  FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) final
  {
    FActorSpawnResult Result;
    SpawnActor(SpawnAtTransform, ActorDescription, Result);
    return Result;
  }

protected:

  UFUNCTION(BlueprintImplementableEvent)
  TArray<FActorDefinition> GenerateDefinitions();

  UFUNCTION(BlueprintImplementableEvent)
  void SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription,
      FActorSpawnResult &SpawnResult);
};
