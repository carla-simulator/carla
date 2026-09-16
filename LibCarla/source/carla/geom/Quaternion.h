// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/geom/Math.h"
#include "carla/geom/RightHandedRotation.h"
#include "carla/geom/RightHandedVector3D.h"
#include "carla/geom/Rotation.h"
#include "carla/geom/Vector3D.h"

#include <cmath>

namespace carla {
namespace geom {

  /// Quaternion representing a 3D rotation, stored as `(x, y, z, w)`.
  ///
  /// **Frame: right-handed**, x forward / y **left** / z up (ROS REP-103
  /// "FLU"). It is the quaternion of `RightHandedRotation`, not of CARLA's
  /// left-handed `Rotation`, and it is bit-for-bit the one the ROS 2 bridge
  /// publishes (`ros2::TransformFromCarlaRotation` in
  /// `carla/ros2/publishers/TransformQuaternion.h`).
  ///
  /// The `Quaternion(const Rotation &)` constructor therefore performs a
  /// handedness change: it routes through `Rotation::ToRightHanded()`, which
  /// negates **yaw and pitch** and leaves roll alone. (The #9751 commit
  /// message says "yaw and roll"; that is wrong -- see
  /// `Docs/coordinate_conventions.md`.) With those signs the basis vectors of
  /// `Quaternion(r)` -- which convert back to CARLA's frame on the way out --
  /// reproduce, column for column, the matrix of `Rotation::RotateVector` and
  /// `Transform::GetMatrix`.
  class Quaternion {
  public:

    // =========================================================================
    // -- Public data members --------------------------------------------------
    // =========================================================================

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    MSGPACK_DEFINE_ARRAY(x, y, z, w);

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    Quaternion() = default;

    constexpr Quaternion(float ix, float iy, float iz, float iw)
      : x(ix),
        y(iy),
        z(iz),
        w(iw) {
    }

    /// Quaternion of a right-handed (FLU) Euler triple, degrees, intrinsic
    /// Z-Y-X: `q = qz(yaw) * qy(pitch) * qx(roll)`. This is the textbook
    /// right-handed roll-pitch-yaw formula -- no sign tricks live here.
    explicit Quaternion(const RightHandedRotation &rotation) {
      const float half_yaw_rad =
          static_cast<float>(Math::ToRadians<double>(rotation.yaw)) * 0.5f;
      const float half_pitch_rad =
          static_cast<float>(Math::ToRadians<double>(rotation.pitch)) * 0.5f;
      const float half_roll_rad =
          static_cast<float>(Math::ToRadians<double>(rotation.roll)) * 0.5f;

      const float cy = std::cos(half_yaw_rad);
      const float sy = std::sin(half_yaw_rad);
      const float cp = std::cos(half_pitch_rad);
      const float sp = std::sin(half_pitch_rad);
      const float cr = std::cos(half_roll_rad);
      const float sr = std::sin(half_roll_rad);

      w = cr * cp * cy + sr * sp * sy;
      x = sr * cp * cy - cr * sp * sy;
      y = cr * sp * cy + sr * cp * sy;
      z = cr * cp * sy - sr * sp * cy;
    }

    /// Quaternion of a CARLA (left-handed) Euler rotation, degrees.
    ///
    /// The handedness change happens exactly once, in
    /// `Rotation::ToRightHanded()`: **yaw and pitch are negated, roll is
    /// not**. The resulting `(x, y, z, w)` is in the right-handed FLU frame
    /// and is the value the ROS 2 bridge publishes.
    explicit Quaternion(const Rotation &rotation)
      : Quaternion(rotation.ToRightHanded()) {
    }

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    /// This rotation as right-handed (FLU) Euler angles, degrees, intrinsic
    /// Z-Y-X. Inverse of `Quaternion(const RightHandedRotation &)`.
    RightHandedRotation ToRightHandedRotation() const {
      const float sin_pitch = 2.0f * (w * y - z * x);
      const float clamped_sin_pitch =
          (sin_pitch > 1.0f) ? 1.0f : (sin_pitch < -1.0f) ? -1.0f : sin_pitch;
      const float pitch_rad = std::asin(clamped_sin_pitch);

      const float roll_rad = std::atan2(
          2.0f * (w * x + y * z),
          1.0f - 2.0f * (x * x + y * y));
      const float yaw_rad = std::atan2(
          2.0f * (w * z + x * y),
          1.0f - 2.0f * (y * y + z * z));

      return RightHandedRotation(
          static_cast<float>(Math::ToDegrees<double>(roll_rad)),
          static_cast<float>(Math::ToDegrees<double>(pitch_rad)),
          static_cast<float>(Math::ToDegrees<double>(yaw_rad)));
    }

    /// Convert this quaternion back to a CARLA (left-handed) Euler rotation.
    /// Inverse of the `Quaternion(Rotation)` constructor; the handedness
    /// change is undone once, by `Rotation::FromRightHanded()`.
    Rotation Rotator() const {
      const float sin_pitch = 2.0f * (w * y - z * x);
      const float clamped_sin_pitch =
          (sin_pitch > 1.0f) ? 1.0f : (sin_pitch < -1.0f) ? -1.0f : sin_pitch;
      const float pitch_rad = std::asin(clamped_sin_pitch);

      const float roll_rad = std::atan2(
          2.0f * (w * x + y * z),
          1.0f - 2.0f * (x * x + y * y));
      const float yaw_rad = std::atan2(
          2.0f * (w * z + x * y),
          1.0f - 2.0f * (y * y + z * z));

      return Rotation::FromRightHanded(RightHandedRotation(
          static_cast<float>(Math::ToDegrees<double>(roll_rad)),
          static_cast<float>(Math::ToDegrees<double>(pitch_rad)),
          static_cast<float>(Math::ToDegrees<double>(yaw_rad))));
    }

    /// Rotate a right-handed vector by this quaternion. Computes
    /// `q * v_pure * q_conjugate` via the Rodrigues form
    /// `v + 2w*(q.xyz x v) + 2*(q.xyz x (q.xyz x v))`.
    RightHandedVector3D RotatedVector(const RightHandedVector3D &in) const {
      const float tx = 2.0f * (y * in.z - z * in.y);
      const float ty = 2.0f * (z * in.x - x * in.z);
      const float tz = 2.0f * (x * in.y - y * in.x);

      const float cx = y * tz - z * ty;
      const float cy = z * tx - x * tz;
      const float cz = x * ty - y * tx;

      return RightHandedVector3D(
          in.x + w * tx + cx,
          in.y + w * ty + cy,
          in.z + w * tz + cz);
    }

    /// Inverse rotation of a right-handed vector. For a unit quaternion this
    /// is equivalent to rotating by the conjugate.
    RightHandedVector3D InverseRotatedVector(const RightHandedVector3D &in) const {
      return Inverse().RotatedVector(in);
    }

    /// Unit vector pointing toward the local X axis of this rotation,
    /// expressed in CARLA's left-handed frame. The canonical axes are
    /// passed through the `Vector3D -> RightHandedVector3D` boundary
    /// conversion so the Y-sign flip happens on both ends of the rotation.
    Vector3D GetForwardVector() const {
      return static_cast<Vector3D>(RotatedVector(Vector3D(1.0f, 0.0f, 0.0f)));
    }

    /// Unit vector pointing toward the local Y axis of this rotation,
    /// expressed in CARLA's left-handed frame.
    Vector3D GetRightVector() const {
      return static_cast<Vector3D>(RotatedVector(Vector3D(0.0f, 1.0f, 0.0f)));
    }

    /// Unit vector pointing toward the local Z axis of this rotation,
    /// expressed in CARLA's left-handed frame.
    Vector3D GetUpVector() const {
      return static_cast<Vector3D>(RotatedVector(Vector3D(0.0f, 0.0f, 1.0f)));
    }

    /// Conjugate of this quaternion. For a unit quaternion the conjugate is
    /// also its inverse.
    Quaternion Conjugate() const {
      return Quaternion(-x, -y, -z, w);
    }

    /// Inverse of this quaternion (assumes non-zero norm).
    Quaternion Inverse() const {
      const float n2 = x * x + y * y + z * z + w * w;
      if (n2 <= 0.0f) {
        return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
      }
      const float k = 1.0f / n2;
      return Quaternion(-x * k, -y * k, -z * k, w * k);
    }

    float Length() const {
      return std::sqrt(x * x + y * y + z * z + w * w);
    }

    Quaternion UnitQuaternion() const {
      const float length = Length();
      if (length <= 0.0f) {
        return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
      }
      const float k = 1.0f / length;
      return Quaternion(x * k, y * k, z * k, w * k);
    }

    /// Hamilton product `(*this) * rhs`.
    Quaternion operator*(const Quaternion &rhs) const {
      return Quaternion(
          w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
          w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
          w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
          w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
    }

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const Quaternion &rhs) const {
      return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
    }

    bool operator!=(const Quaternion &rhs) const {
      return !(*this == rhs);
    }
  };

} // namespace geom
} // namespace carla
