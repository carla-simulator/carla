// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "carla/ros2/types/msg/Time.h"

namespace carla {
namespace ros2 {
namespace msg {

/// autoware_control_msgs/msg/Longitudinal
struct Longitudinal {
  Time stamp;
  Time control_time;
  float velocity = 0.0f;
  float acceleration = 0.0f;
  float jerk = 0.0f;
  bool is_defined_acceleration = false;
  bool is_defined_jerk = false;
};

} // namespace msg
} // namespace ros2
} // namespace carla
