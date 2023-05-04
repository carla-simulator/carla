// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UtilActorFactory.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Util/EmptyActor.h"
#include "Carla/Game/CarlaEpisode.h"

TArray<FActorDefinition> AUtilActorFactory::GetDefinitions()
{
  using ABFL = UActorBlueprintFunctionLibrary;
  auto StaticMeshDefinition = ABFL::MakeGenericDefinition(
      TEXT("util"),
      TEXT("actor"),
      TEXT("empty"));
  StaticMeshDefinition.Class = AEmptyActor::StaticClass();
  
  return { StaticMeshDefinition };
}

FActorSpawnResult AUtilActorFactory::SpawnActor(
    const FTransform &SpawnAtTransform,
    const FActorDescription &ActorDescription)
{
  using ABFL = UActorBlueprintFunctionLibrary;
  auto *World = GetWorld();
  if (World == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT
        ("AUtilActorFactory: cannot spawn mesh into an empty world."));
    return {};
  }

  FActorSpawnParameters SpawnParameters;
  SpawnParameters.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

  auto *StaticMeshActor = World->SpawnActor<AEmptyActor>(
      ActorDescription.Class, SpawnAtTransform, SpawnParameters);

  return FActorSpawnResult(StaticMeshActor);
}
