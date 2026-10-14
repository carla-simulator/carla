// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "carla/ros2/types/msg/Vector3.h"

namespace carla {
namespace ros2 {
namespace msg {

struct CarlaEgoVehicleInfoWheel {
  float tire_friction = 0.0f;
  float damping_rate = 0.0f;
  float max_steer_angle = 0.0f;
  float radius = 0.0f;
  float max_brake_torque = 0.0f;
  float max_handbrake_torque = 0.0f;
  Vector3 position;
};

} // namespace msg
} // namespace ros2
} // namespace carla
