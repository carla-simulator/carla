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

    /// Applies the inverse of this transformation to @a in_point
    void InverseTransformPoint(Vector3D &in_point) const {
      auto out_point = in_point;
      out_point -= location;                   // First translate inverse
      rotation.InverseRotateVector(out_point); // Then rotate inverse
      in_point = out_point;
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
