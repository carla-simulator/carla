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

  // using DatabaseType = std::unordered_map<FActorView::IdType, FActorView>;
  using DatabaseType = TMap<FActorView::IdType, FActorView>;

public:

  using IdType = FActorView::IdType;
  using value_type = FActorView;

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

  FActorView PrepareActorViewForFutureActor(const FActorDescription& ActorDescription);

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
    return Num() == 0;
  }

  bool Contains(uint32 Id) const
  {
    return ActorDatabase.Find(Id) != nullptr;
  }

  FActorView Find(IdType Id) const
  {
    const FActorView* ActorView = ActorDatabase.Find(Id);
    return ActorView ? *ActorView : FActorView();
  }

  FActorView Find(const AActor *Actor) const
  {
    const IdType* PtrToId = Ids.Find(Actor);
    return PtrToId ? Find(*PtrToId) : FActorView();
  }

  FActorView* FindPtr(IdType Id)
  {
    FActorView* ActorView = ActorDatabase.Find(Id);
    return ActorView;
  }

  FActorView* FindPtr(const AActor *Actor)
  {
    IdType* PtrToId = Ids.Find(Actor);
    return PtrToId ? FindPtr(*PtrToId) : nullptr;
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

  auto begin() const noexcept
  {
    return ActorDatabase.begin();
  }

  auto end() const noexcept
  {
    return ActorDatabase.end();
  }

  /// @}
private:

  FActorView MakeView(IdType Id, AActor &Actor, FActorDescription Description) const;

  TMap<IdType, AActor *> Actors;

  TMap<AActor *, IdType> Ids;

  DatabaseType ActorDatabase;

  static IdType ID_COUNTER;
};
