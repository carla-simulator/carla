// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ActorData.h"
#include "FlyingVehicles/MultirotorPawn.h"
#include "Carla/Actor/CarlaActor.h"


void FMultirotorData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  AMultirotorPawn* Multirotor = Cast<AMultirotorPawn>(Actor);
  if (bSimulatePhysics)
  {
    PhysicsControl = Multirotor->GetMultirotorPhysicsControl();
  }
}

void FMultirotorData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  AMultirotorPawn* Multirotor = Cast<AMultirotorPawn>(Actor);
  Multirotor->SetSimulatePhysics(bSimulatePhysics);
  if (bSimulatePhysics)
  {
    Multirotor->ApplyMultirotorPhysicsControl(PhysicsControl);
  }
  Multirotor->ApplyMultirotorControl(Control);
}
