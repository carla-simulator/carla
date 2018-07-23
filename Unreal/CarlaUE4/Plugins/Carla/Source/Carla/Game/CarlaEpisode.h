// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDispatcher.h"

#include "CarlaEpisode.generated.h"

/// A simulation episode.
///
/// Each time the level is restarted a new episode is created.
UCLASS(BlueprintType, Blueprintable)
class CARLA_API UCarlaEpisode : public UObject
{
  GENERATED_BODY()

public:

  void Initialize(const FString &InMapName)
  {
    MapName = InMapName;
  }

  UFUNCTION(BlueprintCallable)
  const FString &GetMapName() const
  {
    return MapName;
  }

  void RegisterActorSpawner(IActorSpawner &ActorSpawner)
  {
    ActorDispatcher.Bind(ActorSpawner);
  }

  /// Return the list of actor definitions that are available to be spawned this
  /// episode.
  UFUNCTION(BlueprintCallable)
  const TArray<FActorDefinition> &GetActorDefinitions() const
  {
    return ActorDispatcher.GetActorDefinitions();
  }

  /// Spawns an actor based on @a ActorDescription at @a Transform. To properly
  /// despawn an actor created with this function call DestroyActor.
  ///
  /// Return nullptr on failure.
  UFUNCTION(BlueprintCallable)
  AActor *SpawnActor(
      const FTransform &Transform,
      const FActorDescription &ActorDescription)
  {
    return ActorDispatcher.SpawnActor(Transform, ActorDescription);
  }

  /// Destroys an actor, properly removing it from the registry.
  UFUNCTION(BlueprintCallable)
  void DestroyActor(AActor *Actor)
  {
    ActorDispatcher.DestroyActor(Actor);
  }

  const FActorRegistry &GetActorRegistry() const
  {
    return ActorDispatcher.GetActorRegistry();
  }

private:

  UPROPERTY(VisibleAnywhere)
  FString MapName;

  FActorDispatcher ActorDispatcher;
};
