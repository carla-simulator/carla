// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorInfo.h"

#include "carla/rpc/ActorState.h"
#include "carla/rpc/AttachmentType.h"

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
    return ::IsValid(TheActor);
  }

  bool IsAlive() const
  {
    return (carla::rpc::ActorState::Alive == State);
  }

  bool IsDormant() const
  {
    return (carla::rpc::ActorState::Dormant == State);
  }

  bool IsPendingKill() const
  {
    return (carla::rpc::ActorState::PendingKill == State);
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

  void SetParent(IdType InParentId)
  {
    ParentId = InParentId;
  }

  IdType GetParent() const
  {
    return ParentId;
  }

  void SetAttachmentType(carla::rpc::AttachmentType InAttachmentType)
  {
    Attachment = InAttachmentType;
  }

  carla::rpc::AttachmentType GetAttachmentType() const
  {
    return Attachment;
  }

private:

  friend class FActorRegistry;

  FActorView(
      IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      carla::rpc::ActorState InState)
    : TheActor(Actor),
      Info(std::move(Info)),
      Id(ActorId),
      State(InState) {}

  AActor *TheActor = nullptr;

  TSharedPtr<const FActorInfo> Info = nullptr;

  IdType Id = 0u;

  IdType ParentId = 0u;

  carla::rpc::ActorState State = carla::rpc::ActorState::Invalid;

  carla::rpc::AttachmentType Attachment = carla::rpc::AttachmentType::INVALID;

  ActorType Type = ActorType::INVALID;

};
