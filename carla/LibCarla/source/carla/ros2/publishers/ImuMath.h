// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <array>
#include <cmath>

namespace carla {
namespace ros2 {

// Returns a yaw-only quaternion (w, x, y, z) corresponding to converting a
// magnetic compass heading (radians, clockwise from North) into ROS REP-103
// yaw (radians, counter-clockwise from East): yaw = pi/2 - compass.
//
// M_PI is used here, not std::numbers::pi_v, because this header is consumed
// by the carla-ros2-native ExternalProject in Ros2Native/, which does not
// configure CMAKE_CXX_STANDARD and therefore defaults to C++17 where
// <numbers> is unavailable.
inline std::array<float, 4> OrientationFromCompass(float compass) {
  const float yaw = static_cast<float>(M_PI) / 2.0f - compass;
  const float c = std::cos(yaw * 0.5f);
  const float s = std::sin(yaw * 0.5f);
  return {c, 0.0f, 0.0f, s};
}

}  // namespace ros2
}  // namespace carla
