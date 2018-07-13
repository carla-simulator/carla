// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "Carla/Actor/ActorRegistry.h"

FActorView UActorRegistry::Register(AActor &Actor)
{
  static IdType ID_COUNTER = 0u;
  auto Id = ++ID_COUNTER;
  Actors.Emplace(Id, &Actor);
  auto Result = ActorDatabase.emplace(Id, FActorView(Id, Actor));
  check(Result.second);
  check(static_cast<size_t>(Actors.Num()) == ActorDatabase.size());
  return Result.first->second;
}

void UActorRegistry::Deregister(IdType Id)
{
  check(Contains(Id));
  ActorDatabase.erase(Id);
  Actors.Remove(Id);
  check(static_cast<size_t>(Actors.Num()) == ActorDatabase.size());
}
