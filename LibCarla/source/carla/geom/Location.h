// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"
#include "carla/geom/Vector3DInt.h"
#include "carla/geom/Math.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <util/enable-ue4-macros.h>
#include "Math/Vector.h"
#include <util/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

  struct Location : Vector3D {

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    Location() = default;

    using Vector3D::Vector3D;

    Location(Vector3D rhs) : Vector3D(rhs) {}

    constexpr Location(Vector3DInt rhs) :
        Vector3D(static_cast<float>(rhs.x),
                 static_cast<float>(rhs.y),
                 static_cast<float>(rhs.z)) {
    }

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    constexpr auto DistanceSquared(Location loc) const {
      return Math::DistanceSquared(*this, loc);
    }

    inline auto Distance(Location loc) const {
      return Math::Distance(*this, loc);
    }

    // =========================================================================
    // -- Arithmetic operators -------------------------------------------------
    // =========================================================================

    constexpr Location &operator+=(Location rhs) {
      static_cast<Vector3D &>(*this) += rhs;
      return *this;
    }

    constexpr friend Location operator+(Location lhs, Location rhs) {
      lhs += rhs;
      return lhs;
    }

    constexpr Location &operator-=(Location rhs) {
      static_cast<Vector3D &>(*this) -= rhs;
      return *this;
    }

    constexpr friend Location operator-(Location lhs, Location rhs) {
      lhs -= rhs;
      return lhs;
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

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

  constexpr bool operator==(Location lhs, Location rhs) {
    return static_cast<Vector3D>(lhs) == static_cast<Vector3D>(rhs);
  }

  constexpr bool operator!=(Location lhs, Location rhs) {
    return !(lhs == rhs);
  }

} // namespace geom
} // namespace carla
