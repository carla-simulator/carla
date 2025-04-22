// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ActorROS2Handler.h"

#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Vehicle/VehicleControl.h"
#include "Carla/Vehicle/VehicleAckermannControl.h"
#include "Carla/Public/ActorDataFallbackComponent.h"

// =============================================================================
// -- Control interface handlers -----------------------------------------------
// =============================================================================

#define PI 	(3.1415926535897932f)

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

void ActorROS2Handler::operator()(carla::ros2::ObstacleReport &Target)
{
  if (!_Actor) return;

  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(_Actor);
  if (!Vehicle) return;

  UActorComponent* actor_comp = _Actor->GetComponentByClass(UActorDataFallbackComponent::StaticClass());
  UActorDataFallbackComponent* data_comp = Cast<UActorDataFallbackComponent>(actor_comp);
  if(!data_comp) return;

  for(const auto& obstacle : data_comp->ROSObstacleData)
  {
    carla::ros2::ObstaclePrediction obs;

    obs.id = obstacle.id;

    float conversion_factor = 0.01;
    
    obs.acc_x = obstacle.acc_x * conversion_factor;
    obs.acc_y = obstacle.acc_y * conversion_factor;
    obs.acc_z = obstacle.acc_z * conversion_factor;

    obs.pos_x = obstacle.pos_x * conversion_factor;
    obs.pos_y = obstacle.pos_y * conversion_factor; 
    obs.pos_z = obstacle.pos_z * conversion_factor;

    obs.speed_x = obstacle.speed_x * conversion_factor;
    obs.speed_y = obstacle.speed_y * conversion_factor;
    obs.speed_z = obstacle.speed_z * conversion_factor;
    
    obs.anchor_x = obstacle.anchor_x * conversion_factor;   
    obs.anchor_y = obstacle.anchor_y * conversion_factor; 
    obs.anchor_z = obstacle.anchor_z * conversion_factor;
    
    obs.corner_points[0]  = obstacle.corner_points[0].x  * conversion_factor;
    obs.corner_points[1]  = obstacle.corner_points[0].y  * conversion_factor;
    obs.corner_points[2]  = obstacle.corner_points[0].z  * conversion_factor;
    obs.corner_points[3]  = obstacle.corner_points[1].x  * conversion_factor;
    obs.corner_points[4]  = obstacle.corner_points[1].y  * conversion_factor;
    obs.corner_points[5]  = obstacle.corner_points[1].z  * conversion_factor;
    obs.corner_points[6]  = obstacle.corner_points[2].x  * conversion_factor;
    obs.corner_points[7]  = obstacle.corner_points[2].y  * conversion_factor;
    obs.corner_points[8]  = obstacle.corner_points[2].z  * conversion_factor;
    obs.corner_points[9]  = obstacle.corner_points[3].x  * conversion_factor;
    obs.corner_points[10] = obstacle.corner_points[3].y  * conversion_factor;
    obs.corner_points[11] = obstacle.corner_points[3].z  * conversion_factor;

    obs.length = obstacle.length  * conversion_factor;
    obs.width = obstacle.width    * conversion_factor;
    obs.height = obstacle.height  * conversion_factor;

    obs.yaw = obstacle.yaw * (PI / 180);

    obs.timestamp = obstacle.timestamp;
    obs.tracking_time = obstacle.tracking_time;

    obs.type = TCHAR_TO_UTF8(*(obstacle.type));

    Target.Obstacles.push_back(obs);
  }
}