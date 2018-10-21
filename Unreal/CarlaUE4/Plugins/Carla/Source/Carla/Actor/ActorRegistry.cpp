// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Game/Tagger.h"

static FString GetRelevantTagAsString(const AActor &Actor)
{
  TArray<ECityObjectLabel> Tags;
  ATagger::GetTagsOfTaggedActor(Actor, Tags);
  for (auto &&Tag : Tags)
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
  auto Result = ActorDatabase.emplace(Id, FActorView(Id, Actor, std::move(Description)));
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
    auto Description = MakeShared<FActorDescription>();
    Description->Id = TEXT("static.") + GetRelevantTagAsString(*Actor);
    View.Description = Description;
    check(View.IsValid());
  }
  return View;
}
