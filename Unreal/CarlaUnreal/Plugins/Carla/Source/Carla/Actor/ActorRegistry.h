// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/CarlaActor.h"

#include <util/ue-header-guard-begin.h>
#include "Containers/Map.h"
#include <util/ue-header-guard-end.h>

#include <unordered_map>

/// A registry of all the Carla actors.
class FActorRegistry
{
public:

  using IdType = FCarlaActor::IdType;
  using ValueType = TSharedPtr<FCarlaActor>;

private:

  // using DatabaseType = std::unordered_map<IdType, FCarlaActor>;
  using DatabaseType = TMap<IdType, TSharedPtr<FCarlaActor>>;

  // ===========================================================================
  /// @name Actor registry functions
  // ===========================================================================
  /// @{
public:

  /// Register the @a Actor in the database. A new ID will be assign to this
  /// actor.
  ///
  /// @warning Undefined if an actor is registered more than once.
  FCarlaActor* Register(AActor &Actor, FActorDescription Description, IdType DesiredId = 0);

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

  FCarlaActor* FindCarlaActor(IdType Id)
  {
    ValueType* CarlaActorPtr = ActorDatabase.Find(Id);
    return CarlaActorPtr ? CarlaActorPtr->Get() : nullptr;
  }

  const FCarlaActor* FindCarlaActor(IdType Id) const
  {
    const ValueType* CarlaActorPtr = ActorDatabase.Find(Id);
    return CarlaActorPtr ? CarlaActorPtr->Get() : nullptr;
  }

  FCarlaActor* FindCarlaActor(const AActor *Actor)
  {
    IdType* PtrToId = Ids.Find(Actor);
    return PtrToId ? FindCarlaActor(*PtrToId) : nullptr;
  }

  const FCarlaActor* FindCarlaActor(const AActor *Actor) const
  {
    const IdType* PtrToId = Ids.Find(Actor);
    return PtrToId ? FindCarlaActor(*PtrToId) : nullptr;
  }

  FString GetDescriptionFromStream(carla::streaming::detail::stream_id_type Id);

  void PutActorToSleep(IdType Id, UCarlaEpisode* CarlaEpisode);

  void WakeActorUp(IdType Id, UCarlaEpisode* CarlaEpisode);

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

  TSharedPtr<FCarlaActor> MakeCarlaActor(
    IdType Id,
    AActor &Actor,
    FActorDescription Description,
    carla::rpc::ActorState InState) const;

  FCarlaActor MakeFakeActor(
    AActor &Actor) const;

  TMap<IdType, AActor *> Actors;

  TMap<AActor *, IdType> Ids;

  DatabaseType ActorDatabase;

  static IdType ID_COUNTER;
};
