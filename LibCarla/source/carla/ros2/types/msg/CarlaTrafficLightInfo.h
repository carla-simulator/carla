// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <cstdint>
#include "carla/ros2/types/msg/CarlaBoundingBox.h"
#include "carla/ros2/types/msg/Pose.h"

namespace carla {
namespace ros2 {
namespace msg {

struct CarlaTrafficLightInfo {
  uint32_t id = 0u;
  Pose transform;
  // trigger_volume center is relative to transform
  CarlaBoundingBox trigger_volume;
};

} // namespace msg
} // namespace ros2
} // namespace carla
