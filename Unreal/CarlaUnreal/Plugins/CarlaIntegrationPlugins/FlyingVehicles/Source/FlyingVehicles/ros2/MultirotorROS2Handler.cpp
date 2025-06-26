// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MultirotorROS2Handler.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Vehicle/VehicleControl.h"

void MultirotorROS2Handler::operator()(carla::ros2::MultirotorControl &Source)
{
  if (!_Actor) return;
  AMultirotorPawn *Multirotor = Cast<AMultirotorPawn>(_Actor);
  if (!Multirotor) return;

  // setup control values
  FMultirotorControl NewControl;
  TArray<float> newThrottle;
  newThrottle.SetNumUninitialized(Source.throttle.size());
  for (int i = 0; i < Source.throttle.size(); i++){
    newThrottle[i] = Source.throttle[i];
  }
  NewControl.Throttle = newThrottle;

  Multirotor->ApplyMultirotorControl(NewControl);
}
