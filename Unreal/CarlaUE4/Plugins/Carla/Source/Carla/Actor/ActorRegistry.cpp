// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "Carla/Actor/ActorRegistry.h"

#include "Carla/Game/Tagger.h"
#include "Carla/Traffic/TrafficLightBase.h"

static FActorView::ActorType FActorRegistry_GetActorType(const FActorView &View)
{
  if (View.IsValid())
  {
    if (nullptr != Cast<ACarlaWheeledVehicle>(View.GetActor()))
    {
      return FActorView::ActorType::Vehicle;
    }
    else if (nullptr != Cast<ACharacter>(View.GetActor()))
    {
      return FActorView::ActorType::Walker;
    }
    else if (nullptr != Cast<ATrafficLightBase>(View.GetActor()))
    {
      return FActorView::ActorType::TrafficLight;
    }
  }
  return FActorView::ActorType::Other;
}

static FString GetRelevantTagAsString(const FActorView &View)
{
  for (auto &&Tag : View.GetSemanticTags())
  {
    if ((Tag != ECityObjectLabel::None) && (Tag != ECityObjectLabel::Other))
    {
      auto Str = ATagger::GetTagAsString(Tag).ToLower();
      return (Str.EndsWith(TEXT("s")) ? Str.LeftChop(1) : Str);
    }
  }
  return TEXT("unknown");
}

FActorView FActorRegistry::Register(AActor &Actor, FActorDescription Description)
{
  static IdType ID_COUNTER = 0u;
  const auto Id = ++ID_COUNTER;
  Actors.Emplace(Id, &Actor);
  if (Ids.Contains(&Actor))
  {
    UE_LOG(
        LogCarla,
        Warning,
        TEXT("This actor's memory address is already registered, "
             "either you forgot to deregister the actor "
             "or the actor was garbage collected."));
  }
  Ids.Emplace(&Actor, Id);

  auto View = FActorView(Id, Actor, std::move(Description));
  ATagger::GetTagsOfTaggedActor(Actor, View.SemanticTags);
  View.Type = FActorRegistry_GetActorType(View);

  auto Result = ActorDatabase.emplace(Id, View);
  check(Result.second);
  check(static_cast<size_t>(Actors.Num()) == ActorDatabase.size());
  return Result.first->second;
}

void FActorRegistry::Deregister(IdType Id)
{
  check(Contains(Id));
  AActor *Actor = FindActor(Id);
  check(Actor != nullptr);
  ActorDatabase.erase(Id);
  Actors.Remove(Id);
  Ids.Remove(Actor);
  check(static_cast<size_t>(Actors.Num()) == ActorDatabase.size());
}

void FActorRegistry::Deregister(AActor *Actor)
{
  auto View = Find(Actor);
  check(View.IsValid());
  Deregister(View.GetActorId());
}

FActorView FActorRegistry::FindOrFake(AActor *Actor) const
{
  if (Actor == nullptr)
  {
    return {};
  }
  auto View = Find(Actor);
  if (!View.IsValid())
  {
    View.TheActor = Actor;
    ATagger::GetTagsOfTaggedActor(*Actor, View.SemanticTags);
    auto Description = MakeShared<FActorDescription>();
    Description->Id = TEXT("static.") + GetRelevantTagAsString(View);
    View.Description = Description;
    check(View.IsValid());
  }
  return View;
}
