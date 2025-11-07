// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/geom/Math.h"
#include "carla/geom/Vector3D.h"
#include "carla/geom/RightHandedVector3D.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Math/Rotator.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {


  /**
   * Class representing the UE4 Rotator (RPY in degrees) 
  */
  class Rotation {
  public:

    // =========================================================================
    // -- Public data members --------------------------------------------------
    // =========================================================================

    float pitch = 0.0f;

    float yaw = 0.0f;

    float roll = 0.0f;

    MSGPACK_DEFINE_ARRAY(pitch, yaw, roll);

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    Rotation() = default;

    Rotation(float p, float y, float r)
      : pitch(p),
        yaw(y),
        roll(r) {}

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    /// Compute the unit vector pointing towards the X-axis of the rotation described by this rotator.
    Vector3D GetForwardVector() const {
      Vector3D const forward_vector(1.0, 0.0, 0.0);
      return RotatedVector(forward_vector);
    }

    /// Compute the unit vector pointing towards the Y-axis of of the rotation described by this rotator.
    Vector3D GetRightVector() const {
      Vector3D const right_vector(0.0, 1.0, 0.0);
      return RotatedVector(right_vector);
    }

    /// Compute the unit vector pointing towards the Z-axis of of the rotation described by this rotator.
    Vector3D GetUpVector() const {
      Vector3D const up_vector(0.0, 0.0, 1.0);
      return RotatedVector(up_vector);
    }

    RightHandedVector3D RotatedVector(RightHandedVector3D const &in_point) const {
      // Rotates Rz(yaw) * Ry(pitch) * Rx(roll) = first x, then y, then z.
      // Unreal uses left handed system: negate x,y,z axis rotations (counter direction), negate y-axis rotation again, because we have a right handed vector at hand now
      const float cr = std::cos(Math::ToRadians(-roll)); // negate
      const float sr = std::sin(Math::ToRadians(-roll)); // negate
      const float cp = std::cos(Math::ToRadians(pitch)); // double-negate
      const float sp = std::sin(Math::ToRadians(pitch)); // double-negate
      const float cy = std::cos(Math::ToRadians(-yaw)); // negate
      const float sy = std::sin(Math::ToRadians(-yaw)); // negate

      // Matrix basis see https://en.wikipedia.org/wiki/Rotation_matrix Euler Angles, alpha=roll, beta=pitch, gamma=yaw
      RightHandedVector3D out_point;
      out_point.x =
        in_point.x * (cp * cy) +
        in_point.y * (cy * sp * sr - sy * cr) +    
        in_point.z * (cy * sp * cr + sy * sr);

      out_point.y =
        in_point.x * (cp * sy) +
        in_point.y * (sy * sp * sr + cy * cr) +
        in_point.z * (sy * sp * cr - cy * sr);

      out_point.z =
        in_point.x * (-sp) +
        in_point.y * (cp * sr) +
        in_point.z * (cp * cr);

      return out_point;
    }

    RightHandedVector3D InverseRotatedVector(RightHandedVector3D const &in_point) const {
      // Unreal uses left handed system: negate x,y,z axis rotations (counter direction), negate y-axis rotation again, because we have a right handed vector at hand now
      const float cr = std::cos(Math::ToRadians(-roll)); // negate
      const float sr = std::sin(Math::ToRadians(-roll)); // negate
      const float cp = std::cos(Math::ToRadians(pitch)); // double-negate
      const float sp = std::sin(Math::ToRadians(pitch)); // double-negate
      const float cy = std::cos(Math::ToRadians(-yaw)); // negate
      const float sy = std::sin(Math::ToRadians(-yaw)); // negate

      // Applies the transposed of the matrix used in RotateVector function,
      // which is the rotation inverse.
      RightHandedVector3D out_point;
      out_point.x =
        in_point.x * (cp * cy) +
        in_point.y * (cp * sy) +
        in_point.z * (-sp);

      out_point.y =
        in_point.x * (cy * sp * sr - sy * cr) +
        in_point.y * (sy * sp * sr + cy * cr) +
        in_point.z * (cp * sr);

      out_point.z =
        in_point.x * (cy * sp * cr + sy * sr) +
        in_point.y * (sy * sp * cr - cy * sr) +
        in_point.z * (cp * cr);

      return out_point;
    }

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const Rotation &rhs) const  {
      return (pitch == rhs.pitch) && (yaw == rhs.yaw) && (roll == rhs.roll);
    }

    bool operator!=(const Rotation &rhs) const  {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- Conversions to UE4 types ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Rotation(const FRotator &rotator)
      : Rotation(rotator.Pitch, rotator.Yaw, rotator.Roll) {}

    operator FRotator() const {
      return FRotator{pitch, yaw, roll};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace geom
} // namespace carla