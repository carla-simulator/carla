// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <cstdint>

namespace carla {
namespace ros2 {
namespace msg {

struct RegionOfInterest {
  uint32_t x_offset = 0;
  uint32_t y_offset = 0;
  uint32_t height = 0;
  uint32_t width = 0;
  bool do_rectify = false;
};

} // namespace msg
} // namespace ros2
} // namespace carla
