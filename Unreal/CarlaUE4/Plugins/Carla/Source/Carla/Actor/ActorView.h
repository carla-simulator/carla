// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorInfo.h"

#include "carla/rpc/ActorState.h"

#include "ActorView.generated.h"

class AActor;

/// A view over an actor and its properties.
USTRUCT()
struct FActorView
{
  GENERATED_BODY()
public:

  using IdType = uint32;

  enum class ActorType : uint8
  {
    Other,
    Vehicle,
    Walker,
    TrafficLight,
    TrafficSign,
    INVALID
  };

  FActorView() = default;
  // FActorView(const FActorView &) = default;
  // FActorView(FActorView &&) = default;

  bool IsValid() const
  {
    return IsDormant() || ((TheActor != nullptr) && !TheActor->IsPendingKill());
  }

  bool IsDormant() const
  {
    return (carla::rpc::ActorState::Dormant == State);
  }

  IdType GetActorId() const
  {
    return Id;
  }

  ActorType GetActorType() const
  {
    return Type;
  }

  AActor *GetActor()
  {
    return TheActor;
  }

  const AActor *GetActor() const
  {
    return TheActor;
  }

  const FActorInfo *GetActorInfo() const
  {
    return Info.Get();
  }

  carla::rpc::ActorState GetActorState() const
  {
    return State;
  }

  void SetActorState(carla::rpc::ActorState InState)
  {
    State = InState;
  }

private:

  friend class FActorRegistry;

  FActorView(IdType ActorId, AActor* Actor, TSharedPtr<const FActorInfo> Info)
    : TheActor(Actor),
      Info(std::move(Info)),
      Id(ActorId) {}

  AActor *TheActor = nullptr;

  TSharedPtr<const FActorInfo> Info = nullptr;

  IdType Id = 0u;

  carla::rpc::ActorState State = carla::rpc::ActorState::Alive;

  ActorType Type = ActorType::Other;

};
