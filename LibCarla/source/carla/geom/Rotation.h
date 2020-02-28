// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/geom/Math.h"
#include "carla/geom/Vector3D.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Math/Rotator.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

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

    Vector3D GetForwardVector() const {
      return Math::GetForwardVector(*this);
    }

    Vector3D GetRightVector() const {
      return Math::GetRightVector(*this);
    }

    void RotateVector(Vector3D &in_point) const {
      // Rotates Rz(yaw) * Ry(pitch) * Rx(roll) = first x, then y, then z.
      const float cy = std::cos(Math::ToRadians(yaw));
      const float sy = std::sin(Math::ToRadians(yaw));
      const float cr = std::cos(Math::ToRadians(roll));
      const float sr = std::sin(Math::ToRadians(roll));
      const float cp = std::cos(Math::ToRadians(pitch));
      const float sp = std::sin(Math::ToRadians(pitch));

      Vector3D out_point;
      out_point.x =
        in_point.x * (cp * cy) +
        in_point.y * (cy * sp * sr - sy * cr) +
        in_point.z * (-cy * sp * cr - sy * sr);

      out_point.y =
        in_point.x * (cp * sy) +
        in_point.y * (sy * sp * sr + cy * cr) +
        in_point.z * (-sy * sp * cr + cy * sr);

      out_point.z =
        in_point.x * (sp) +
        in_point.y * (-cp * sr) +
        in_point.z * (cp * cr);

      in_point = out_point;
    }

    void InverseRotateVector(Vector3D &in_point) const {
      // Applies the transposed of the matrix used in RotateVector function,
      // which is the rotation inverse.
      const float cy = std::cos(Math::ToRadians(yaw));
      const float sy = std::sin(Math::ToRadians(yaw));
      const float cr = std::cos(Math::ToRadians(roll));
      const float sr = std::sin(Math::ToRadians(roll));
      const float cp = std::cos(Math::ToRadians(pitch));
      const float sp = std::sin(Math::ToRadians(pitch));

      Vector3D out_point;
      out_point.x =
        in_point.x * (cp * cy) +
        in_point.y * (cp * sy) +
        in_point.z * (sp);

      out_point.y =
        in_point.x * (cy * sp * sr - sy * cr) +
        in_point.y * (sy * sp * sr + cy * cr) +
        in_point.z * (-cp * sr);

      out_point.z =
        in_point.x * (-cy * sp * cr - sy * sr) +
        in_point.y * (-sy * sp * cr + cy * sr) +
        in_point.z * (cp * cr);

      in_point = out_point;
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
