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
#include <compiler/enable-ue4-macros.h>
#include "Math/Vector.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

  /**
   * Stores the acceleration vector of an entity in m/s²
  */
  class Acceleration : public Vector3D {
  public:

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    Acceleration() = default;

    using Vector3D::Vector3D;

    Acceleration(const Vector3D &rhs) : Vector3D(rhs) {}

    Acceleration(const Vector3DInt &rhs) :
        Vector3D(static_cast<float>(rhs.x),
                 static_cast<float>(rhs.y),
                 static_cast<float>(rhs.z)) {}

    // =========================================================================
    // -- Arithmetic operators -------------------------------------------------
    // =========================================================================

    Acceleration &operator+=(const Acceleration &rhs) {
      static_cast<Vector3D &>(*this) += static_cast<const Vector3D &>(rhs);
      return *this;
    }

    friend Acceleration operator+(Acceleration lhs, const Acceleration &rhs) {
      lhs += rhs;
      return lhs;
    }

    Acceleration &operator-=(const Acceleration &rhs) {
      static_cast<Vector3D &>(*this) -= static_cast<const Vector3D &>(rhs);
      return *this;
    }

    friend Acceleration operator-(Acceleration lhs, const Acceleration &rhs) {
      lhs -= rhs;
      return lhs;
    }

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const Acceleration &rhs) const {
      return static_cast<const Vector3D &>(*this) == static_cast<const Vector3D &>(rhs);
    }

    bool operator!=(const Acceleration &rhs) const {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- Conversions to UE4 types ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    // from cm/s² to m/s².
    Acceleration(const FVector &vector)
      : Acceleration(1e-2f * vector.X, 1e-2f * vector.Y, 1e-2f * vector.Z) {}

    // from m/s² to cm/s²
    operator FVector() const {
      return FVector{1e2f * x, 1e2f * y, 1e2f * z}; 
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace geom
} // namespace carla
