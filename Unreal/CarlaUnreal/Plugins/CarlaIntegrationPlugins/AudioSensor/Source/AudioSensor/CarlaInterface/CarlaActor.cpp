// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaActor.h"
#include "AudioSensor/AudioSensorActor.h"

const FString FAudioSensorActor::CustomType = "AudioSensor_AudioSensor";

FAudioSensorActor::FAudioSensorActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState,
    UWorld* World)
    : FCarlaActor(ActorId, Actor, Info, InState, World)
{
  Type = ActorType::Custom;
  ActorData = MakeShared<FAudioSensorData>();
}

TSharedPtr<FAudioSensorActor> FAudioSensorActor::CreateInstance(FCarlaActor::IdType ActorId, 
                AActor* Actor, 
                TSharedPtr<const FActorInfo> Info, 
                carla::rpc::ActorState InState, 
                UWorld* World)
  {
    return MakeShared<FAudioSensorActor>(ActorId, Actor, std::move(Info), InState, World);
  }

void FAudioSensorActor::RegisterClassWithFactory()
  {
    CarlaActorConstructorFactory::Instance().Register(TEXT("FAudioSensorActor"), AAudioSensorActor::StaticClass(), CreateInstance);
  }
