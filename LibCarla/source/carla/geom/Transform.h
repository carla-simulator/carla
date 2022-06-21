// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/geom/Rotation.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include "Math/Transform.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

  class Transform {
  public:

    // =========================================================================
    // -- Public data members --------------------------------------------------
    // =========================================================================

    Location location;

    Rotation rotation;

    MSGPACK_DEFINE_ARRAY(location, rotation);

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    Transform() = default;

    Transform(const Location &in_location)
      : location(in_location),
        rotation() {}

    Transform(const Location &in_location, const Rotation &in_rotation)
      : location(in_location),
        rotation(in_rotation) {}

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    Vector3D GetForwardVector() const {
      return rotation.GetForwardVector();
    }

    Vector3D GetRightVector() const {
      return rotation.GetRightVector();
    }

    Vector3D GetUpVector() const {
      return rotation.GetUpVector();
    }

    /// Applies this transformation to @a in_point (first translation then rotation).
    void TransformPoint(Vector3D &in_point) const {
      auto out_point = in_point;
      rotation.RotateVector(out_point); // First rotate
      out_point += location;            // Then translate
      in_point = out_point;
    }

    /// Applies this transformation to @a in_vector (rotation only).
    void TransformVector(Vector3D &in_vector) const {
      auto out_vector = in_vector;
      rotation.RotateVector(out_vector); // First rotate
      in_vector = out_vector;
    }

    /// Applies the inverse of this transformation to @a in_point
    void InverseTransformPoint(Vector3D &in_point) const {
      auto out_point = in_point;
      out_point -= location;                   // First translate inverse
      rotation.InverseRotateVector(out_point); // Then rotate inverse
      in_point = out_point;
    }

    /// Computes the 4-matrix form of the transformation
    std::array<float, 16> GetMatrix() const {
      const float yaw = rotation.yaw;
      const float cy = std::cos(Math::ToRadians(yaw));
      const float sy = std::sin(Math::ToRadians(yaw));

      const float roll = rotation.roll;
      const float cr = std::cos(Math::ToRadians(roll));
      const float sr = std::sin(Math::ToRadians(roll));

      const float pitch = rotation.pitch;
      const float cp = std::cos(Math::ToRadians(pitch));
      const float sp = std::sin(Math::ToRadians(pitch));

      std::array<float, 16> transform = {
          cp * cy, cy * sp * sr - sy * cr, -cy * sp * cr - sy * sr, location.x,
          cp * sy, sy * sp * sr + cy * cr, -sy * sp * cr + cy * sr, location.y,
          sp, -cp * sr, cp * cr, location.z,
          0.0, 0.0, 0.0, 1.0};

      return transform;
    }

    /// Computes the 4-matrix form of the inverse transformation
    std::array<float, 16> GetInverseMatrix() const {
      const float yaw = rotation.yaw;
      const float cy = std::cos(Math::ToRadians(yaw));
      const float sy = std::sin(Math::ToRadians(yaw));

      const float roll = rotation.roll;
      const float cr = std::cos(Math::ToRadians(roll));
      const float sr = std::sin(Math::ToRadians(roll));

      const float pitch = rotation.pitch;
      const float cp = std::cos(Math::ToRadians(pitch));
      const float sp = std::sin(Math::ToRadians(pitch));

      Vector3D a = {0.0f, 0.0f, 0.0f};
      InverseTransformPoint(a);

      std::array<float, 16> transform = {
          cp * cy, cp * sy, sp, a.x,
          cy * sp * sr - sy * cr, sy * sp * sr + cy * cr, -cp * sr, a.y,
          -cy * sp * cr - sy * sr, -sy * sp * cr + cy * sr, cp * cr, a.z,
          0.0f, 0.0f, 0.0f, 1.0};

      return transform;
    }

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const Transform &rhs) const {
      return (location == rhs.location) && (rotation == rhs.rotation);
    }

    bool operator!=(const Transform &rhs) const {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- Conversions to UE4 types ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Transform(const FTransform &transform)
      : Transform(Location(transform.GetLocation()), Rotation(transform.Rotator())) {}

    operator FTransform() const {
      const FVector scale{1.0f, 1.0f, 1.0f};
      return FTransform{FRotator(rotation), FVector(location), scale};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace geom
} // namespace carla
