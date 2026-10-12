// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "carla/ros2/types/msg/Time.h"

namespace carla {
namespace ros2 {
namespace msg {

/// autoware_control_msgs/msg/Lateral
struct Lateral {
  Time stamp;
  Time control_time;
  float steering_tire_angle = 0.0f;
  float steering_tire_rotation_rate = 0.0f;
  bool is_defined_steering_tire_rotation_rate = false;
};

} // namespace msg
} // namespace ros2
} // namespace carla
