// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace geom {

  /// Euler angles of a rotation expressed in a **right-handed** frame, in
  /// degrees. This is the boundary type for ROS / REP-103 "FLU" consumers:
  /// x forward, y **left**, z up, every axis turned by the right-hand rule.
  ///
  /// The three angles compose as the usual intrinsic Z-Y-X (yaw, then pitch,
  /// then roll) sequence, i.e. `R = Rz(yaw) * Ry(pitch) * Rx(roll)` with the
  /// standard right-handed elementary matrices. That is exactly what
  /// `scipy.spatial.transform.Rotation.from_euler("xyz", ...)`,
  /// `tf2::Quaternion::setRPY` and NVIDIA's ClipGT `rig.json` all mean by
  /// "roll pitch yaw".
  ///
  /// This is **not** `carla::geom::Rotation`. CARLA's frame is left-handed
  /// (x forward, y **right**, z up) and its rotator has mixed signs; the
  /// exact mapping is
  ///
  ///     right_handed.roll  =  carla.roll
  ///     right_handed.pitch = -carla.pitch
  ///     right_handed.yaw   = -carla.yaw
  ///
  /// which is its own inverse. Use `Rotation::ToRightHanded()` /
  /// `Rotation::FromRightHanded()` rather than flipping signs by hand, and
  /// see `Docs/coordinate_conventions.md`.
  struct RightHandedRotation {

    /// Rotation about the forward (+X) axis, degrees, right-hand rule.
    float roll = 0.0f;

    /// Rotation about the left (+Y) axis, degrees, right-hand rule.
    float pitch = 0.0f;

    /// Rotation about the up (+Z) axis, degrees, right-hand rule.
    float yaw = 0.0f;

    RightHandedRotation() = default;

    constexpr RightHandedRotation(float in_roll, float in_pitch, float in_yaw)
      : roll(in_roll),
        pitch(in_pitch),
        yaw(in_yaw) {
    }

    constexpr bool operator==(const RightHandedRotation &rhs) const {
      return (roll == rhs.roll) && (pitch == rhs.pitch) && (yaw == rhs.yaw);
    }

    constexpr bool operator!=(const RightHandedRotation &rhs) const {
      return !(*this == rhs);
    }
  };

} // namespace geom
} // namespace carla
