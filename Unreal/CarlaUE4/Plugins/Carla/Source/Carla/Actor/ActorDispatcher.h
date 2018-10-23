// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Actor/ActorSpawnResult.h"

#include "Containers/Array.h"
#include "Templates/Function.h"

class ACarlaActorFactory;

/// Actor in charge of binding ActorDefinitions to spawn functions, as well as
/// keeping the registry of all the actors spawned.
class FActorDispatcher
{
public:

  using SpawnFunctionType = TFunction<FActorSpawnResult(const FTransform &, const FActorDescription &)>;

  /// Bind a definition to a spawn function. When SpawnActor is called with a
  /// matching description @a Functor is called.
  ///
  /// @warning Invalid definitions are ignored.
  void Bind(FActorDefinition Definition, SpawnFunctionType SpawnFunction);

  /// Bind all the definitions of @a ActorFactory to its spawn function.
  ///
  /// @warning Invalid definitions are ignored.
  void Bind(ACarlaActorFactory &ActorFactory);

  /// Spawns an actor based on @a ActorDescription at @a Transform. To properly
  /// despawn an actor created with this function call DestroyActor.
  ///
  /// @return A pair containing the result of the spawn function and a view over
  /// the actor and its properties. If the status is different of Success the
  /// view is invalid.
  TPair<EActorSpawnResultStatus, FActorView> SpawnActor(
      const FTransform &Transform,
      FActorDescription ActorDescription);

  /// Destroys an actor, properly removing it from the registry.
  ///
  /// Return true if the @a Actor is destroyed or already marked for
  /// destruction, false if indestructible or nullptr.
  bool DestroyActor(AActor *Actor);

  const TArray<FActorDefinition> &GetActorDefinitions() const
  {
    return Definitions;
  }

  FActorRegistry &GetActorRegistry()
  {
    return Registry;
  }

  const FActorRegistry &GetActorRegistry() const
  {
    return Registry;
  }

private:

  TArray<FActorDefinition> Definitions;

  TArray<SpawnFunctionType> SpawnFunctions;

  TArray<TSubclassOf<AActor>> Classes;

  FActorRegistry Registry;
};
