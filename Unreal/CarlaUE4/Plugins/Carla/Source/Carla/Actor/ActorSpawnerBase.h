// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"

#include "Containers/Array.h"

#include <algorithm>

#include "ActorSpawnerBase.generated.h"

/// Interface for the actor spawner. It is implemented in blueprints.
UCLASS(BlueprintType, Blueprintable)
class CARLA_API UActorSpawnerBase : public UObject
{
  GENERATED_BODY()

public:

  UFUNCTION(BlueprintImplementableEvent)
  TArray<FActorDefinition> GenerateDefinitions();

  UFUNCTION(BlueprintImplementableEvent)
  void SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription,
      AActor *&Actor);
};
