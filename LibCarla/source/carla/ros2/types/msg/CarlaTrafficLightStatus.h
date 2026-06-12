// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <cstdint>

namespace carla {
namespace ros2 {
namespace msg {

struct CarlaTrafficLightStatus {
  // state constants per carla_msgs/CarlaTrafficLightStatus:
  // RED=0, YELLOW=1, GREEN=2, OFF=3, UNKNOWN=4
  uint32_t id = 0u;
  uint8_t state = 0u;
};

} // namespace msg
} // namespace ros2
} // namespace carla
