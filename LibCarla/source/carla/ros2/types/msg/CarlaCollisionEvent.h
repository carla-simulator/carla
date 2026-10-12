// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <cstdint>
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/Vector3.h"

namespace carla {
namespace ros2 {
namespace msg {

struct CarlaCollisionEvent {
  Header header;
  uint32_t other_actor_id = 0;
  Vector3 normal_impulse;
};

} // namespace msg
} // namespace ros2
} // namespace carla
