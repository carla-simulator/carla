// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/ActorDispatcher.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/CarlaActorFactory.h"

#include "Carla/Game/Tagger.h"

#include "GameFramework/Controller.h"

void UActorDispatcher::Bind(FActorDefinition Definition, SpawnFunctionType Functor)
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

void UActorDispatcher::Bind(ACarlaActorFactory &ActorFactory)
{
  for (const auto &Definition : ActorFactory.GetDefinitions())
  {
    Bind(Definition, [&](const FTransform &Transform, const FActorDescription &Description) {
      return ActorFactory.SpawnActor(Transform, Description);
    });
  }
}

TPair<EActorSpawnResultStatus, FActorView> UActorDispatcher::SpawnActor(
    const FTransform &Transform,
    FActorDescription Description,
    FActorView::IdType DesiredId)
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

  auto View = Result.IsValid() ? RegisterActor(*Result.Actor, std::move(Description), DesiredId) : FActorView();

  if (!View.IsValid())
  {
    UE_LOG(LogCarla, Warning, TEXT("Failed to spawn actor '%s'"), *Description.Id);
    check(Result.Status != EActorSpawnResultStatus::Success);
  }
  else
  {
    ATagger::TagActor(*View.GetActor(), true);
  }

  return MakeTuple(Result.Status, View);
}

bool UActorDispatcher::DestroyActor(AActor *Actor)
{
  if (Actor == nullptr) {
    UE_LOG(LogCarla, Error, TEXT("Trying to destroy nullptr actor"));
    return false;
  }

  // Check if the actor is in the registry.
  auto View = Registry.Find(Actor);
  if (!View.IsValid()) {
    UE_LOG(LogCarla, Warning, TEXT("Trying to destroy actor that is not in the registry"));
    return false;
  }
  const auto &Id = View.GetActorInfo()->Description.Id;

  // Destroy its controller if present.
  auto Pawn = Cast<APawn>(Actor);
  auto Controller = (Pawn != nullptr ? Pawn->GetController() : nullptr);
  if (Controller != nullptr)
  {
    UE_LOG(LogCarla, Log, TEXT("Destroying actor's controller: '%s'"), *Id);
    auto Success = Controller->Destroy();
    if (!Success)
    {
      UE_LOG(LogCarla, Error, TEXT("Failed to destroy actor's controller: '%s'"), *Id);
    }
  }

  // Destroy the actor.
  UE_LOG(LogCarla, Log, TEXT("Destroying actor: '%s'"), *Id);
  if (Actor->Destroy())
  {
    return true;
  }
  UE_LOG(LogCarla, Error, TEXT("Failed to destroy actor: '%s'"), *Id);
  return false;
}

FActorView UActorDispatcher::RegisterActor(AActor &Actor, FActorDescription Description, FActorRegistry::IdType DesiredId)
{
  auto View = Registry.Register(Actor, std::move(Description), DesiredId);
  if (View.IsValid())
  {
    Actor.OnDestroyed.AddDynamic(this, &UActorDispatcher::OnActorDestroyed);
  }
  return View;
}
