// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "carla/ros2/types/msg/Header.h"

namespace carla {
namespace ros2 {
namespace msg {

struct VelocityReport {
  Header header;
  float longitudinal_velocity = 0.0f;
  float lateral_velocity = 0.0f;
  float heading_rate = 0.0f;
};

} // namespace msg
} // namespace ros2
} // namespace carla
