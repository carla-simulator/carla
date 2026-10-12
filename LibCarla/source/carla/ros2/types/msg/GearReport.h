// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <cstdint>

#include "carla/ros2/types/msg/Time.h"

namespace carla {
namespace ros2 {
namespace msg {

struct GearReport {
  static const uint8_t NONE = 0;
  static const uint8_t NEUTRAL = 1;
  static const uint8_t DRIVE = 2;
  static const uint8_t DRIVE_2 = 3;
  static const uint8_t DRIVE_3 = 4;
  static const uint8_t DRIVE_4 = 5;
  static const uint8_t DRIVE_5 = 6;
  static const uint8_t DRIVE_6 = 7;
  static const uint8_t DRIVE_7 = 8;
  static const uint8_t DRIVE_8 = 9;
  static const uint8_t DRIVE_9 = 10;
  static const uint8_t DRIVE_10 = 11;
  static const uint8_t DRIVE_11 = 12;
  static const uint8_t DRIVE_12 = 13;
  static const uint8_t DRIVE_13 = 14;
  static const uint8_t DRIVE_14 = 15;
  static const uint8_t DRIVE_15 = 16;
  static const uint8_t DRIVE_16 = 17;
  static const uint8_t DRIVE_17 = 18;
  static const uint8_t DRIVE_18 = 19;
  static const uint8_t REVERSE = 20;
  static const uint8_t REVERSE_2 = 21;
  static const uint8_t PARK = 22;
  static const uint8_t LOW = 23;
  static const uint8_t LOW_2 = 24;

  Time stamp;
  uint8_t report = 0;
};

} // namespace msg
} // namespace ros2
} // namespace carla
