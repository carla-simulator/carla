// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorRegistry.h"

#include "Containers/Array.h"
#include "Templates/Function.h"

class IActorSpawner;

/// Actor in charge of binding ActorDefinitions to spawn functions, as well as
/// keeping the registry of all the actors spawned.
class FActorDispatcher
{
public:

  using SpawnFunctionType = TFunction<AActor*(const FTransform &, const FActorDescription &)>;

  /// Bind a definition to a spawn function. When SpawnActor is called with a
  /// matching description @a Functor is called.
  ///
  /// @warning Invalid definitions are ignored.
  void Bind(FActorDefinition Definition, SpawnFunctionType SpawnFunction);

  /// Bind all the definitions of @a ActorSpawner to its spawn function.
  ///
  /// @warning Invalid definitions are ignored.
  void Bind(IActorSpawner &ActorSpawner);

  /// Spawns an actor based on @a ActorDescription at @a Transform. To properly
  /// despawn an actor created with this function call DestroyActor.
  ///
  /// Return nullptr on failure.
  AActor *SpawnActor(
      const FTransform &Transform,
      const FActorDescription &ActorDescription);

  /// Destroys an actor, properly removing it from the registry.
  void DestroyActor(AActor *Actor);

  const TArray<FActorDefinition> &GetActorDefinitions() const
  {
    return Definitions;
  }

  const FActorRegistry &GetActorRegistry() const
  {
    return Registry;
  }

private:

  TArray<FActorDefinition> Definitions;

  TArray<SpawnFunctionType> SpawnFunctions;

  FActorRegistry Registry;
};
