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

// Converts a linear acceleration sampled in CARLA's left-handed sensor frame
// (x forward, y right, z up) into the ROS REP-103 right-handed sensor frame
// (x forward, y left, z up). Ordinary (polar) vectors only need the y axis
// mirrored — the same y-negation our TF, lidar and odometry publishers apply.
inline std::array<float, 3> AccelToRos(float ax, float ay, float az) {
  return {ax, -ay, az};
}

// Converts an angular velocity sampled in CARLA's left-handed sensor frame
// into the REP-103 right-handed sensor frame.
//
// Angular velocity is a pseudovector, so mirroring the y axis flips the sign
// of the components PARALLEL to the mirror plane (x and z), not the mirrored
// one. Concretely, with UE's component cross product v = w x r evaluated in
// left-handed coordinates:
//   - turning left  (CCW from above)  -> UE wz < 0, ROS wz > 0  => negate z
//   - rolling right (right side down) -> UE wx < 0, ROS wx > 0  => negate x
//   - pitching nose-down              -> UE wy > 0, ROS wy > 0  => keep y
// This matches the carla-ros-bridge ImuSensor conversion (-x, +y, -z). Note
// tier4's autoware-support layer negated (y, z) instead, which mis-handles
// the pseudovector reflection; we deliberately diverge.
inline std::array<float, 3> GyroToRos(float gx, float gy, float gz) {
  return {-gx, gy, -gz};
}

}  // namespace ros2
}  // namespace carla
