// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorView.h"

#include "Containers/Map.h"

#include <unordered_map>

/// A registry of all the Carla actors.
class FActorRegistry
{
private:

  using DatabaseType = std::unordered_map<FActorView::IdType, FActorView>;

public:

  using IdType = DatabaseType::key_type;

  // ===========================================================================
  /// @name Actor registry functions
  // ===========================================================================
  /// @{
public:

  /// Register the @a Actor in the database. A new ID will be assign to this
  /// actor.
  ///
  /// @warning Undefined if an actor is registered more than once.
  FActorView Register(AActor &Actor);

  void Deregister(IdType Id);

  void Deregister(AActor *Actor);

  /// @}
  // ===========================================================================
  /// @name Look up functions
  // ===========================================================================
  /// @{

  int32 Num() const
  {
    return Actors.Num();
  }

  bool IsEmpty() const
  {
    return ActorDatabase.empty();
  }

  bool Contains(uint32 Id) const
  {
    return ActorDatabase.find(Id) != ActorDatabase.end();
  }

  FActorView Find(IdType Id) const
  {
    auto it = ActorDatabase.find(Id);
    return it != ActorDatabase.end() ? it->second : FActorView();
  }

  FActorView Find(AActor *Actor) const
  {
    auto PtrToId = Ids.Find(Actor);
    return PtrToId != nullptr ? Find(*PtrToId) : FActorView();
  }

  AActor *FindActor(IdType Id) const
  {
    auto View = Find(Id);
    return View.IsValid() ? View.GetActor() : nullptr;
  }

  /// @}
  // ===========================================================================
  /// @name Range iteration support
  // ===========================================================================
  /// @{
public:

  using key_type = DatabaseType::key_type;
  using mapped_type = DatabaseType::mapped_type;
  using value_type = DatabaseType::value_type;
  using const_iterator = DatabaseType::const_iterator;

  const_iterator begin() const noexcept
  {
    return ActorDatabase.begin();
  }

  const_iterator end() const noexcept
  {
    return ActorDatabase.end();
  }

  /// @}
private:

  TMap<IdType, AActor *> Actors;

  TMap<AActor *, IdType> Ids;

  DatabaseType ActorDatabase;
};
