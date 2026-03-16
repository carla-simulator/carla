#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "carla/ros2/types/msg/Header.h"

namespace carla {
namespace ros2 {
namespace msg {

struct Image {
  Header header;
  uint32_t height = 0;
  uint32_t width = 0;
  std::string encoding;
  uint8_t is_bigendian = 0;
  uint32_t step = 0;
  std::vector<uint8_t> data;
};

} // namespace msg
} // namespace ros2
} // namespace carla
