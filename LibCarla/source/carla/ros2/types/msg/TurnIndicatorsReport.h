// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <cstdint>

#include "carla/ros2/types/msg/Time.h"

namespace carla {
namespace ros2 {
namespace msg {

struct TurnIndicatorsReport {
  static const uint8_t DISABLE = 1;
  static const uint8_t ENABLE_LEFT = 2;
  static const uint8_t ENABLE_RIGHT = 3;

  Time stamp;
  uint8_t report = 0;
};

} // namespace msg
} // namespace ros2
} // namespace carla
