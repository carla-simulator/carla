// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ActorView.h"


void FActorView::BuildActorData()
{
  switch(Type)
  {
  case ActorType::TrafficSign:
    ActorData = MakeShared<FTrafficSignData>();
    break;
  case ActorType::TrafficLight:
    ActorData = MakeShared<FTrafficLightData>();
    break;
  case ActorType::Vehicle:
    ActorData = MakeShared<FVehicleData>();
    break;
  case ActorType::Walker:
  ActorData = MakeShared<FWalkerData>();
    break;
  default:
    ActorData = MakeShared<FActorData>();
    break;
  }
}

void FActorView::PutActorToSleep(UCarlaEpisode* CarlaEpisode)
{
  State = carla::rpc::ActorState::Dormant;
  if (ActorData)
  {
    ActorData->RecordActorData(TheActor, CarlaEpisode);
  }
  TheActor->Destroy();
  TheActor = nullptr;
}

void FActorView::WakeActorUp(UCarlaEpisode* CarlaEpisode)
{
  TheActor = ActorData->RespawnActor(CarlaEpisode, *Info);
  if (TheActor == nullptr)
  {
    State = carla::rpc::ActorState::Invalid;
    UE_LOG(LogCarla, Error, TEXT("Could not wake up dormant actor %d at location %s"), GetActorId(), *(ActorData->GetLocalTransform(CarlaEpisode).GetLocation().ToString()));
    return;
  }
  State = carla::rpc::ActorState::Alive;
  ActorData->RestoreActorData(TheActor, CarlaEpisode);
}
