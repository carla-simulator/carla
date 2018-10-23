// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/ActorDispatcher.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/CarlaActorFactory.h"

void FActorDispatcher::Bind(FActorDefinition Definition, SpawnFunctionType Functor)
{
  if (UActorBlueprintFunctionLibrary::CheckActorDefinition(Definition))
  {
    Definition.UId = static_cast<uint32>(SpawnFunctions.Num()) + 1u;
    Definitions.Emplace(Definition);
    SpawnFunctions.Emplace(Functor);
    Classes.Emplace(Definition.Class);
  }
  else
  {
    UE_LOG(LogCarla, Warning, TEXT("Invalid definition '%s' ignored"), *Definition.Id);
  }
}

void FActorDispatcher::Bind(ACarlaActorFactory &ActorFactory)
{
  for (const auto &Definition : ActorFactory.GetDefinitions())
  {
    Bind(Definition, [&](const FTransform &Transform, const FActorDescription &Description) {
      return ActorFactory.SpawnActor(Transform, Description);
    });
  }
}

TPair<EActorSpawnResultStatus, FActorView> FActorDispatcher::SpawnActor(
    const FTransform &Transform,
    FActorDescription Description)
{
  if ((Description.UId == 0u) || (Description.UId > static_cast<uint32>(SpawnFunctions.Num())))
  {
    UE_LOG(LogCarla, Error, TEXT("Invalid ActorDescription '%s' (UId=%d)"), *Description.Id, Description.UId);
    return MakeTuple(EActorSpawnResultStatus::InvalidDescription, FActorView());
  }

  UE_LOG(LogCarla, Log, TEXT("Spawning actor '%s'"), *Description.Id);

  Description.Class = Classes[Description.UId - 1];
  auto Result = SpawnFunctions[Description.UId - 1](Transform, Description);

  if ((Result.Status == EActorSpawnResultStatus::Success) && (Result.Actor == nullptr))
  {
    UE_LOG(LogCarla, Warning, TEXT("ActorSpawnResult: Trying to spawn '%s'"), *Description.Id);
    UE_LOG(LogCarla, Warning, TEXT("ActorSpawnResult: Reported success but did not return an actor"));
    Result.Status = EActorSpawnResultStatus::UnknownError;
  }

  auto View = Result.IsValid() ? Registry.Register(*Result.Actor, std::move(Description)) : FActorView();

  if (!View.IsValid())
  {
    UE_LOG(LogCarla, Warning, TEXT("Failed to spawn actor '%s'"), *Description.Id);
    check(Result.Status != EActorSpawnResultStatus::Success);
  }

  return MakeTuple(Result.Status, View);
}

bool FActorDispatcher::DestroyActor(AActor *Actor)
{
  if ((Actor != nullptr) && Actor->Destroy())
  {
    Registry.Deregister(Actor);
    return true;
  }
  return false;
}
