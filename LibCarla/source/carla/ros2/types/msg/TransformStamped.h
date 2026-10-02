// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <string>
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/Transform.h"

namespace carla {
namespace ros2 {
namespace msg {

struct TransformStamped {
  Header header;
  std::string child_frame_id;
  Transform transform;
};

} // namespace msg
} // namespace ros2
} // namespace carla
