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

// Output of TransformFromCarlaRotation: a translation (tx, ty, tz) and a
// (w, x, y, z) quaternion encoding a CARLA actor's world transform in the
// ROS 2 right-handed-Z-up convention.
struct TransformQuaternion {
  std::array<float, 3> translation;
  std::array<float, 4> rotation;  // (w, x, y, z)
};

// Converts CARLA (pitch, yaw, roll) Euler angles in **degrees** plus an
// (tx, ty, tz) translation into a ROS-handed (translation, quaternion)
// pair. This is the math previously inlined inside
// CarlaTransformPublisher::ComputeTransform; extracting it lets a GTest pin
// the deg-to-radian conversion + per-axis sign flip without pulling the
// FastDDS publisher into the Build-Tests/ compile unit.
//
// Notes:
//   - degree-to-radian conversion uses M_PI / 180, restored by PR-1
//     (`d9cfc79fb`) after a prior spelling shipped `M_PI_2 / 180` and halved
//     every rotation;
//   - pitch and yaw axes carry a leading sign flip to land in the ROS
//     handedness; roll is left as-is;
//   - the translation Y axis is mirrored to match the same handedness
//     change.
//
// M_PI is used here, not std::numbers::pi_v, because this header is consumed
// by the carla-ros2-native ExternalProject in Ros2Native/, which does not
// configure CMAKE_CXX_STANDARD and therefore defaults to C++17 where
// <numbers> is unavailable.
[[nodiscard]] inline TransformQuaternion TransformFromCarlaRotation(
    float tx, float ty, float tz,
    float pitch_deg, float yaw_deg, float roll_deg) noexcept {
  constexpr float deg2rad = static_cast<float>(M_PI) / 180.0f;
  const float rx = -pitch_deg * deg2rad;
  const float ry = -yaw_deg * deg2rad;
  const float rz =  roll_deg * deg2rad;

  const float cr = std::cos(rz * 0.5f);
  const float sr = std::sin(rz * 0.5f);
  const float cp = std::cos(rx * 0.5f);
  const float sp = std::sin(rx * 0.5f);
  const float cy = std::cos(ry * 0.5f);
  const float sy = std::sin(ry * 0.5f);

  return {
      {tx, -ty, tz},
      {
          cr * cp * cy + sr * sp * sy,
          sr * cp * cy - cr * sp * sy,
          cr * sp * cy + sr * cp * sy,
          cr * cp * sy - sr * sp * cy,
      },
  };
}

}  // namespace ros2
}  // namespace carla
