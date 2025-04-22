// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ActorROS2Handler.h"

#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Vehicle/VehicleControl.h"
#include "Carla/Vehicle/VehicleAckermannControl.h"

// =============================================================================
// -- Control interface handlers -----------------------------------------------
// =============================================================================

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

// =============================================================================
// -- Pseudo publisher handlers ------------------------------------------------
// =============================================================================

void ActorROS2Handler::operator()(carla::ros2::VehicleOdometryReport &Target)
{
  if (!_Actor) return;

  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(_Actor);
  if (!Vehicle) return;

  auto Transform = Vehicle->GetVehicleTransform();

  auto Location = Transform.GetLocation();
  auto Rotation = Transform.Rotator();

  Target.x = Location.X;
  Target.y = Location.Y;
  Target.z = Location.Z;
  Target.roll = Rotation.Roll;
  Target.pitch = Rotation.Pitch;
  Target.yaw = Rotation.Yaw;

  auto Velocity = Vehicle->GetVelocity();
  auto AngularVelocity = Vehicle->GetAngularVelocity();

  Target.linear_velocity_x = Velocity.X;  // [cm/s]
  Target.linear_velocity_y = Velocity.Y;
  Target.linear_velocity_z = Velocity.Z;
  Target.angular_velocity_x = AngularVelocity.X; // Degrees / s
  Target.angular_velocity_y = AngularVelocity.Y;
  Target.angular_velocity_z = AngularVelocity.Z;
}

void ActorROS2Handler::operator()(carla::ros2::VehicleChassisReport & Target)
{
  if (!_Actor) return;

  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(_Actor);
  if (!Vehicle) return;

  //auto Telemetry = Vehicle->GetVehicleTelemetryData();
  auto LastControl = Vehicle->GetVehicleControl();

  Target.engine_started = true; // For now set it to true always
  Target.engine_rpm = 0;
  Target.speed_mps = Vehicle->GetVehicleForwardSpeed() / 100.0f; // [m/s]
  Target.throttle_percentage = LastControl.Throttle * 100.0f;  // last applied. Alguna forma de coger el actual?
  Target.brake_percentage = LastControl.Brake * 100.0f;        // last applied. Alguna forma de coger el actual?
  Target.steering_percentage = -LastControl.Steer * 100.0f;     // last applied. Alguna forma de coger el actual?
}
