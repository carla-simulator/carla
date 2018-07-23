// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorSpawnResult.h"

#include "Containers/Array.h"
#include "GameFramework/Actor.h"

/// Interface for Carla actor spawners.
class IActorSpawner
{
public:

  virtual ~IActorSpawner() {}

  /// Retrieve the list of actor definitions that this class is able to spawn.
  virtual TArray<FActorDefinition> MakeDefinitions() = 0;

  /// Spawn an actor based on @a ActorDescription and @a Transform.
  ///
  /// @pre ActorDescription is expected to be derived from one of the
  /// definitions retrieved with MakeDefinitions.
  virtual FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) = 0;
};
