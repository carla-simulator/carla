// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Math.h"
#include "carla/geom/Quaternion.h"
#include "carla/geom/RightHandedRotation.h"
#include "carla/geom/RightHandedVector3D.h"

#include <array>
#include <cmath>

namespace carla {
namespace geom {

  /// A rigid transform expressed in a **right-handed** frame: x forward,
  /// y **left**, z up (ROS REP-103 "FLU"), metres and degrees.
  ///
  /// This is the explicit boundary type between CARLA's left-handed world and
  /// every right-handed consumer (ROS 2 / tf2, NVIDIA ClipGT rigs, scipy,
  /// Open3D, ...). Build one with `Transform::ToRightHanded()` and go back
  /// with `Transform::FromRightHanded()`; the handedness change then happens
  /// exactly once, here, instead of being sprinkled over call sites.
  ///
  /// Relative to the corresponding `Transform`, with `S = diag(1, -1, 1)`:
  ///
  ///   * `location  = (x, -y, z)`
  ///   * `rotation  = (roll, -pitch, -yaw)`, intrinsic Z-Y-X, right-hand rule
  ///   * `GetMatrix() == S * Transform::GetMatrix() * S` (with the 4x4 `S`)
  ///
  /// Example: `Rotation(pitch=20, yaw=30, roll=10)` maps to
  /// `RightHandedRotation(roll=10, pitch=-20, yaw=-30)`.
  ///
  /// See `Docs/coordinate_conventions.md`.
  struct RightHandedTransform {

    /// Translation in the right-handed frame, metres.
    RightHandedVector3D location;

    /// Orientation in the right-handed frame: roll, pitch, yaw in degrees,
    /// right-hand rule, applied as intrinsic Z-Y-X.
    RightHandedRotation rotation;

    RightHandedTransform() = default;

    constexpr RightHandedTransform(
        const RightHandedVector3D &in_location,
        const RightHandedRotation &in_rotation)
      : location(in_location),
        rotation(in_rotation) {
    }

    explicit constexpr RightHandedTransform(const RightHandedVector3D &in_location)
      : location(in_location),
        rotation() {
    }

    /// Orientation as a right-handed `(x, y, z, w)` quaternion. Identical to
    /// what `ros2::TransformFromCarlaRotation` publishes for the equivalent
    /// CARLA rotation (that helper orders the components `(w, x, y, z)`).
    Quaternion GetQuaternion() const {
      return Quaternion(rotation);
    }

    /// Row-major 4x4 pose matrix in the right-handed frame,
    /// `R = Rz(yaw) * Ry(pitch) * Rx(roll)` with the standard right-handed
    /// elementary matrices and `location` in the last column.
    ///
    /// Equal, element for element, to `S * Transform::GetMatrix() * S` with
    /// `S = diag(1, -1, 1, 1)` -- the similarity transform that mirrors the Y
    /// axis. That identity is pinned by
    /// `test_geom.cpp::right_handed_matrix_is_s_m_s`.
    std::array<float, 16> GetMatrix() const {
      const float cy = std::cos(Math::ToRadians(rotation.yaw));
      const float sy = std::sin(Math::ToRadians(rotation.yaw));
      const float cp = std::cos(Math::ToRadians(rotation.pitch));
      const float sp = std::sin(Math::ToRadians(rotation.pitch));
      const float cr = std::cos(Math::ToRadians(rotation.roll));
      const float sr = std::sin(Math::ToRadians(rotation.roll));

      return {
          cy * cp, cy * sp * sr - sy * cr, cy * sp * cr + sy * sr, location.x,
          sy * cp, sy * sp * sr + cy * cr, sy * sp * cr - cy * sr, location.y,
          -sp,     cp * sr,                cp * cr,                location.z,
          0.0f,    0.0f,                   0.0f,                   1.0f};
    }

    bool operator==(const RightHandedTransform &rhs) const {
      return (location.x == rhs.location.x) &&
             (location.y == rhs.location.y) &&
             (location.z == rhs.location.z) &&
             (rotation == rhs.rotation);
    }

    bool operator!=(const RightHandedTransform &rhs) const {
      return !(*this == rhs);
    }
  };

} // namespace geom
} // namespace carla
