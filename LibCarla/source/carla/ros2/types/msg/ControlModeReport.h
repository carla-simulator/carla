// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <cstdint>

#include "carla/ros2/types/msg/Time.h"

namespace carla {
namespace ros2 {
namespace msg {

struct ControlModeReport {
  static const uint8_t NO_COMMAND = 0;
  static const uint8_t AUTONOMOUS = 1;
  static const uint8_t AUTONOMOUS_STEER_ONLY = 2;
  static const uint8_t AUTONOMOUS_VELOCITY_ONLY = 3;
  static const uint8_t MANUAL = 4;
  static const uint8_t DISENGAGED = 5;
  static const uint8_t NOT_READY = 6;

  Time stamp;
  uint8_t mode = 0;
};

} // namespace msg
} // namespace ros2
} // namespace carla
