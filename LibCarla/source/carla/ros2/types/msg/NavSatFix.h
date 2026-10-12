// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <array>
#include <cstdint>
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/NavSatStatus.h"

namespace carla {
namespace ros2 {
namespace msg {

struct NavSatFix {
  static const uint8_t COVARIANCE_TYPE_UNKNOWN = 0;
  static const uint8_t COVARIANCE_TYPE_APPROXIMATED = 1;
  static const uint8_t COVARIANCE_TYPE_DIAGONAL_KNOWN = 2;
  static const uint8_t COVARIANCE_TYPE_KNOWN = 3;

  Header header;
  NavSatStatus status;
  double latitude = 0.0;
  double longitude = 0.0;
  double altitude = 0.0;
  std::array<double, 9> position_covariance = {};
  uint8_t position_covariance_type = 0;
};

} // namespace msg
} // namespace ros2
} // namespace carla
