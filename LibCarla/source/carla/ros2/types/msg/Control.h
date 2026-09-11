// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "carla/ros2/types/msg/Lateral.h"
#include "carla/ros2/types/msg/Longitudinal.h"
#include "carla/ros2/types/msg/Time.h"

namespace carla {
namespace ros2 {
namespace msg {

/// autoware_control_msgs/msg/Control
struct Control {
  Time stamp;
  Time control_time;
  Lateral lateral;
  Longitudinal longitudinal;
};

} // namespace msg
} // namespace ros2
} // namespace carla
