// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaEgoVehicleInfoPublisher.h"

#include "carla/ros2/publishers/UeToRosConversions.h"

namespace carla {
namespace ros2 {

bool CarlaEgoVehicleInfoPublisher::Write(
    uint32_t id,
    const std::string &type_id,
    const std::string &role_name,
    const geom::Transform &vehicle_transform,
    const rpc::VehiclePhysicsControl &physics_control) {
  auto message = _impl->GetMessage();

  message->id = id;
  message->type = type_id;
  message->rolename = role_name;

  message->wheels.clear();
  message->wheels.reserve(physics_control.wheels.size());
  for (const auto &wheel : physics_control.wheels) {
    msg::CarlaEgoVehicleInfoWheel wheel_info;
    wheel_info.tire_friction = wheel.tire_friction;
    wheel_info.damping_rate = wheel.damping_rate;
    wheel_info.max_steer_angle = wheel.max_steer_angle * UE_DEG_TO_RAD;
    wheel_info.radius = wheel.radius;
    wheel_info.max_brake_torque = wheel.max_brake_torque;
    wheel_info.max_handbrake_torque = wheel.max_handbrake_torque;

    // Wheel positions arrive as world coordinates in centimeters; express
    // them in the vehicle frame in meters, then flip to right-handed.
    geom::Vector3D wheel_position{
        wheel.position.x / 100.0f,
        wheel.position.y / 100.0f,
        wheel.position.z / 100.0f};
    vehicle_transform.InverseTransformPoint(wheel_position);
    wheel_info.position.x = wheel_position.x;
    wheel_info.position.y = -wheel_position.y;
    wheel_info.position.z = wheel_position.z;

    message->wheels.push_back(wheel_info);
  }

  message->max_rpm = physics_control.max_rpm;
  message->moi = physics_control.moi;
  message->damping_rate_full_throttle = physics_control.damping_rate_full_throttle;
  message->damping_rate_zero_throttle_clutch_engaged =
      physics_control.damping_rate_zero_throttle_clutch_engaged;
  message->damping_rate_zero_throttle_clutch_disengaged =
      physics_control.damping_rate_zero_throttle_clutch_disengaged;
  message->use_gear_autobox = physics_control.use_gear_autobox;
  message->gear_switch_time = physics_control.gear_switch_time;
  message->clutch_strength = physics_control.clutch_strength;
  message->mass = physics_control.mass;
  message->drag_coefficient = physics_control.drag_coefficient;
  message->center_of_mass.x = physics_control.center_of_mass.x;
  message->center_of_mass.y = physics_control.center_of_mass.y;
  message->center_of_mass.z = physics_control.center_of_mass.z;

  return true;
}

}  // namespace ros2
}  // namespace carla
