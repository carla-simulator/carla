#pragma once
#include <cstdint>
#include <string>

namespace carla {
namespace ros2 {
namespace msg {

struct TF2Error {
  uint8_t error = 0;
  std::string error_string;
};

} // namespace msg
} // namespace ros2
} // namespace carla
