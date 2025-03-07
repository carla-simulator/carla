// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaActor.h"
#include "CameraControllerActorData.h"

const FString FCameraControllerActor::CustomType = "CameraController_CameraController";

FCameraControllerActor::FCameraControllerActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState,
    UWorld* World)
    : FCarlaActor(ActorId, Actor, Info, InState, World)
{
  Type = ActorType::Custom;
  ActorData = MakeShared<FCameraControllerData>();
}

TSharedPtr<FCameraControllerActor> FCameraControllerActor::CreateInstance(
    FCarlaActor::IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState,
    UWorld* World)
  {
    return MakeShared<FCameraControllerActor>(ActorId, Actor, std::move(Info), InState, World);
  }

void FCameraControllerActor::RegisterClassWithFactory()
  {
    CarlaActorConstructorFactory::Instance().Register(TEXT("FCameraControllerActor"), ACameraControllerActor::StaticClass(), CreateInstance);
  }
