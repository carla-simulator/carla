// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorInfo.h"
#include "Carla/Actor/ActorData.h"

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
    Sensor,
    INVALID
  };

  FActorView() = default;
  // FActorView(const FActorView &) = default;
  // FActorView(FActorView &&) = default;

  bool IsValid() const
  {
    return (carla::rpc::ActorState::Invalid != State);
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

  void AddChildren(IdType ChildId)
  {
    Children.Add(ChildId);
  }

  void RemoveChildren(IdType ChildId)
  {
    Children.Remove(ChildId);
  }

  const TArray<IdType>& GetChildren() const
  {
    return Children;
  }

  void SetAttachmentType(carla::rpc::AttachmentType InAttachmentType)
  {
    Attachment = InAttachmentType;
  }

  carla::rpc::AttachmentType GetAttachmentType() const
  {
    return Attachment;
  }

  void BuildActorData();

  void PutActorToSleep(UCarlaEpisode* CarlaEpisode);

  void WakeActorUp(UCarlaEpisode* CarlaEpisode);

  FActorData* GetActorData()
  {
    return ActorData.Get();
  }

  const FActorData* GetActorData() const
  {
    return ActorData.Get();
  }

  template<typename T>
  T* GetActorData()
  {
    return dynamic_cast<T*>(ActorData.Get());
  }

  template<typename T>
  const T* GetActorData() const
  {
    return dynamic_cast<T*>(ActorData.Get());
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

  TArray<IdType> Children;

  ActorType Type = ActorType::INVALID;

  TSharedPtr<FActorData> ActorData = nullptr;

};
