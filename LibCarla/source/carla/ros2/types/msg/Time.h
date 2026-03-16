#pragma once
#include <cstdint>

namespace carla {
namespace ros2 {
namespace msg {

struct Time {
  int32_t sec = 0;
  uint32_t nanosec = 0;
};

} // namespace msg
} // namespace ros2
} // namespace carla
