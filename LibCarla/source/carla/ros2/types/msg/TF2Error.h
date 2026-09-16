// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

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
