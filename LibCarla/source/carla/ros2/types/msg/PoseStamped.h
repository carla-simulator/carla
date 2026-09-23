// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/Pose.h"

namespace carla {
namespace ros2 {
namespace msg {

struct PoseStamped {
  Header header;
  Pose pose;
};

} // namespace msg
} // namespace ros2
} // namespace carla
