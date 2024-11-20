// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorSpawnResult.h"

#include <util/ue-header-guard-begin.h>
#include "Containers/Array.h"
#include "GameFramework/Actor.h"
#include <util/ue-header-guard-end.h>

#include "CarlaActorFactory.generated.h"

/// Base class for Carla actor factories.
UCLASS(Abstract)
class CARLA_API ACarlaActorFactory : public AActor
{
  GENERATED_BODY()

public:

  ACarlaActorFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
  {
    PrimaryActorTick.bCanEverTick = false;
  }

  /// Retrieve the list of actor definitions that this class is able to spawn.
  virtual TArray<FActorDefinition> GetDefinitions() {
    unimplemented();
    return {};
  }

  /// Spawn an actor based on @a ActorDescription and @a Transform.
  ///
  /// @pre ActorDescription is expected to be derived from one of the
  /// definitions retrieved with GetDefinitions.
  virtual FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) {
    unimplemented();
    return {};
  }
};
