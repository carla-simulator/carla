// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"
#include "carla/geom/Vector3DInt.h"
#include "carla/geom/Math.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Math/Vector.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

  class Location : public Vector3D {
  public:

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    Location() = default;

    using Vector3D::Vector3D;

    Location(const Vector3D &rhs) : Vector3D(rhs) {}

    Location(const Vector3DInt &rhs) :
        Vector3D(static_cast<float>(rhs.x),
                 static_cast<float>(rhs.y),
                 static_cast<float>(rhs.z)) {}

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    auto Distance(const Location &loc) const {
      return Math::Distance(*this, loc);
    }

    // =========================================================================
    // -- Arithmetic operators -------------------------------------------------
    // =========================================================================

    Location &operator+=(const Location &rhs) {
      static_cast<Vector3D &>(*this) += rhs;
      return *this;
    }

    friend Location operator+(Location lhs, const Location &rhs) {
      lhs += rhs;
      return lhs;
    }

    Location &operator-=(const Location &rhs) {
      static_cast<Vector3D &>(*this) -= rhs;
      return *this;
    }

    friend Location operator-(Location lhs, const Location &rhs) {
      lhs -= rhs;
      return lhs;
    }

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const Location &rhs) const {
      return static_cast<const Vector3D &>(*this) == rhs;
    }

    bool operator!=(const Location &rhs) const {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- Conversions to UE4 types ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Location(const FVector &vector) // from centimeters to meters.
      : Location(1e-2f * vector.X, 1e-2f * vector.Y, 1e-2f * vector.Z) {}

    operator FVector() const {
      return FVector{1e2f * x, 1e2f * y, 1e2f * z}; // from meters to centimeters.
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace geom
} // namespace carla
