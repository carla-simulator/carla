#pragma once
#include <cstdint>
#include <vector>
#include "carla/ros2/types/msg/Header.h"

namespace carla {
namespace ros2 {
namespace msg {

struct CarlaLineInvasion {
  Header header;
  std::vector<int32_t> crossed_lane_markings;
};

} // namespace msg
} // namespace ros2
} // namespace carla
