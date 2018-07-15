// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawner.h"

#include "ActorSpawnerBlueprintBase.generated.h"

/// Base class for Blueprints implementing IActorSpawner interface.
///
/// Blueprints deriving from this class are expected to override
/// GenerateDefinitions and SpawnActor functions.
UCLASS(BlueprintType, Blueprintable)
class CARLA_API AActorSpawnerBlueprintBase
  : public AActor,
    public IActorSpawner
{
  GENERATED_BODY()

public:

  virtual TArray<FActorDefinition> MakeDefinitions() final
  {
    return GenerateDefinitions();
  }

  virtual AActor *SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) final
  {
    AActor *Actor = nullptr;
    SpawnActor(SpawnAtTransform, ActorDescription, Actor);
    return Actor;
  }

protected:

  UFUNCTION(BlueprintImplementableEvent)
  TArray<FActorDefinition> GenerateDefinitions();

  UFUNCTION(BlueprintImplementableEvent)
  void SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription,
      AActor *&Actor);
};
