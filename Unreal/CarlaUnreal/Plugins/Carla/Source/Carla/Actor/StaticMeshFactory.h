// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawnResult.h"
#include "Carla/Actor/CarlaActorFactory.h"

#include "StaticMeshFactory.generated.h"

/// Factory in charge of spawning static meshes. This factory is able to spawn
/// any mesh in content.
UCLASS()
class CARLA_API AStaticMeshFactory : public ACarlaActorFactory
{
  GENERATED_BODY()

  /// Retrieve the definitions of the static mesh actor
  TArray<FActorDefinition> GetDefinitions() final;

  FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) final;

private:
  /// Lazy, memoizing mesh cache keyed by mesh path. This factory takes an
  /// arbitrary mesh path at spawn time (there is no catalog to pre-populate
  /// from), so the cache fills on first spawn of a given path and spares
  /// subsequent spawns of the same path the synchronous LoadObject hit.
  UPROPERTY(Transient, DuplicateTransient)
  TMap<FString, TObjectPtr<UStaticMesh>> MeshCacheByPath;
};
