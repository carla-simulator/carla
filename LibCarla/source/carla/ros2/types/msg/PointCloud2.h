// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <cstdint>
#include <vector>
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/PointField.h"

namespace carla {
namespace ros2 {
namespace msg {

struct PointCloud2 {
  Header header;
  uint32_t height = 0;
  uint32_t width = 0;
  std::vector<PointField> fields;
  bool is_bigendian = false;
  uint32_t point_step = 0;
  uint32_t row_step = 0;
  std::vector<uint8_t> data;
  bool is_dense = false;
};

} // namespace msg
} // namespace ros2
} // namespace carla
