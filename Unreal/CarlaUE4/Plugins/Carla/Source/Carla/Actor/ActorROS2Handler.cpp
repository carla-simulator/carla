// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ActorROS2Handler.h"

#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Vehicle/VehicleControl.h"
#include "Carla/Vehicle/VehicleAckermannControl.h"

void ActorROS2Handler::operator()(carla::ros2::VehicleControl &Source)
{
  if (!_Actor) return;

  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(_Actor);
  if (!Vehicle) return;

  // setup control values
  FVehicleControl NewControl;
  NewControl.Throttle = Source.throttle;
  NewControl.Steer = Source.steer;
  NewControl.Brake = Source.brake;
  NewControl.bHandBrake = Source.hand_brake;
  NewControl.bReverse = Source.reverse;
  NewControl.bManualGearShift = Source.manual_gear_shift;
  NewControl.Gear = Source.gear;

  Vehicle->ApplyVehicleControl(NewControl, EVehicleInputPriority::User);
}

void ActorROS2Handler::operator()(carla::ros2::AckermannControl &Source)
{
  if (!_Actor) return;

  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(_Actor);
  if (!Vehicle) return;

  // setup control values
  FVehicleAckermannControl NewControl;
  NewControl.Steer = Source.steer;
  NewControl.SteerSpeed = Source.steer_speed;
  NewControl.Speed = Source.speed;
  NewControl.Acceleration = Source.acceleration;
  NewControl.Jerk = Source.jerk;

  Vehicle->ApplyVehicleAckermannControl(NewControl, EVehicleInputPriority::User);
}
