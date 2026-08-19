// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "carla/ros2/types/msg/Time.h"

namespace carla {
namespace ros2 {
namespace msg {

struct SteeringReport {
  Time stamp;
  float steering_tire_angle = 0.0f;
};

} // namespace msg
} // namespace ros2
} // namespace carla
