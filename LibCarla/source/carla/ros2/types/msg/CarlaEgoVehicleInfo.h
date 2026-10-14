// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "carla/ros2/types/msg/CarlaEgoVehicleInfoWheel.h"
#include "carla/ros2/types/msg/Vector3.h"

namespace carla {
namespace ros2 {
namespace msg {

struct CarlaEgoVehicleInfo {
  uint32_t id = 0u;
  std::string type;
  std::string rolename;
  std::vector<CarlaEgoVehicleInfoWheel> wheels;
  float max_rpm = 0.0f;
  float moi = 0.0f;
  float damping_rate_full_throttle = 0.0f;
  float damping_rate_zero_throttle_clutch_engaged = 0.0f;
  float damping_rate_zero_throttle_clutch_disengaged = 0.0f;
  bool use_gear_autobox = false;
  float gear_switch_time = 0.0f;
  float clutch_strength = 0.0f;
  float mass = 0.0f;
  float drag_coefficient = 0.0f;
  Vector3 center_of_mass;
};

} // namespace msg
} // namespace ros2
} // namespace carla
