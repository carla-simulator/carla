// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorView.h"

#include "Containers/Map.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Iterator.h>
#include <compiler/enable-ue4-macros.h>

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
  FActorView Register(AActor &Actor, FActorDescription Description, IdType DesiredId = 0);

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

  FActorView Find(const AActor *Actor) const
  {
    auto PtrToId = Ids.Find(Actor);
    return PtrToId != nullptr ? Find(*PtrToId) : FActorView();
  }

  /// If the actor is not found in the registry, create a fake actor view. The
  /// returned FActorView has some information about the @a Actor but will have
  /// an invalid id.
  FActorView FindOrFake(AActor *Actor) const;

  /// @}
  // ===========================================================================
  /// @name Range iteration support
  // ===========================================================================
  /// @{
public:

  using value_type = DatabaseType::mapped_type;

  auto begin() const noexcept
  {
    return carla::iterator::make_map_values_const_iterator(ActorDatabase.begin());
  }

  auto end() const noexcept
  {
    return carla::iterator::make_map_values_const_iterator(ActorDatabase.end());
  }

  /// @}
private:

  FActorView MakeView(IdType Id, AActor &Actor, FActorDescription Description) const;

  TMap<IdType, AActor *> Actors;

  TMap<AActor *, IdType> Ids;

  DatabaseType ActorDatabase;
};
