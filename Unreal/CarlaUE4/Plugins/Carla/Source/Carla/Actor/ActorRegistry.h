// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorView.h"

#include "Containers/Map.h"

#include <unordered_map>

#include "ActorRegistry.generated.h"

/// A registry of all the Carla actors.
UCLASS(BlueprintType, Blueprintable)
class CARLA_API UActorRegistry : public UObject
{
  GENERATED_BODY()

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

  FActorView Find(IdType Id) const
  {
    auto it = ActorDatabase.find(Id);
    return it != ActorDatabase.end() ? it->second : FActorView();
  }

  /// @}
  // ===========================================================================
  /// @name Blueprint support
  // ===========================================================================
  /// @{
public:

  // UPROPERTY(BlueprintCallable)
  int32 Num() const
  {
    return Actors.Num();
  }

  // UPROPERTY(BlueprintCallable)
  bool IsEmpty() const
  {
    return ActorDatabase.empty();
  }

  // UPROPERTY(BlueprintCallable)
  bool Contains(IdType Id) const
  {
    return ActorDatabase.find(Id) != ActorDatabase.end();
  }

  // UPROPERTY(BlueprintCallable)
  AActor *FindActor(IdType Id) const
  {
    auto PtrToPtr = Actors.Find(Id);
    return PtrToPtr != nullptr ? *PtrToPtr : nullptr;
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

  // Because UPROPERTY doesn't understand aliases...
  static_assert(sizeof(IdType) == sizeof(uint32), "Id type missmatch!");

  // This one makes sure actors are not garbage collected.
  UPROPERTY(VisibleAnywhere)
  TMap<uint32, AActor *> Actors;

  DatabaseType ActorDatabase;
};
