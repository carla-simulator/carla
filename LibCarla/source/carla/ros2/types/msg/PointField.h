// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <cstdint>
#include <string>

namespace carla {
namespace ros2 {
namespace msg {

struct PointField {
  static const uint8_t INT8 = 1;
  static const uint8_t UINT8 = 2;
  static const uint8_t INT16 = 3;
  static const uint8_t UINT16 = 4;
  static const uint8_t INT32 = 5;
  static const uint8_t UINT32 = 6;
  static const uint8_t FLOAT32 = 7;
  static const uint8_t FLOAT64 = 8;

  std::string name;
  uint32_t offset = 0;
  uint8_t datatype = 0;
  uint32_t count = 0;
};

} // namespace msg
} // namespace ros2
} // namespace carla
