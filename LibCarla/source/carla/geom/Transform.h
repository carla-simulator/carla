// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/geom/Quaternion.h"
#include "carla/geom/Rotation.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Math/Transform.h"
#include <compiler/disable-ue4-macros.h>
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

    explicit Transform(const Location &in_location)
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
      Vector3D out_point = rotation.RotatedVector(in_point); // First rotate
      in_point = out_point + Vector3D(location);  // Then translate
    }

    /// Applies this transformation to @a in_vector (rotation only).
    void TransformVector(Vector3D &in_vector) const {
      in_vector = rotation.RotatedVector(in_vector); // Only rotation
    }

    /// Applies the inverse of this transformation to @a in_point
    void InverseTransformPoint(Vector3D &in_point) const {
      Vector3D out_point = in_point - Vector3D(location);  // First translate inverse
      in_point = rotation.InverseRotatedVector(out_point); // Then rotate inverse
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

#if ALLOW_UNSAFE_GEOM_MATRIX_ACCESS
    // this matrix is rotating within a right handed-coordinate system, but Unreal coordinate frame is left-handed!
    // If we want to make this public, a dedicated Matrix4x4 type has to be defined considiering that (see also Quaternion.h)
    std::array<float, 16> TransformationMatrix() const {
      auto const matrix = Quaternion(rotation).RotationMatrix();

      std::array<float, 16> transform = {
          matrix[0], matrix[1], matrix[2], location.x,
          matrix[3], matrix[4], matrix[5], location.y,
          matrix[6], matrix[7], matrix[8], location.z,
          0.0, 0.0, 0.0, 1.0};

      return transform;
    }

    /// Computes the 4-matrix form of the inverse transformation
    std::array<float, 16> InverseTransformationMatrix() const {
      auto const matrix = Quaternion(rotation).InverseRotationMatrix();

      Vector3D a = {0.0f, 0.0f, 0.0f};
      InverseTransformPoint(a);

      std::array<float, 16> transform = {
          matrix[0], matrix[1], matrix[2], a.x,
          matrix[3], matrix[4], matrix[5], a.y,
          matrix[6], matrix[7], matrix[8], a.z,
          0.0f, 0.0f, 0.0f, 1.0};

      return transform;
    }
#endif
  };

} // namespace geom
} // namespace carla
