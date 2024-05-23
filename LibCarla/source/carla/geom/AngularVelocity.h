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
   * Stores the angular velocity vector in deg/s
  */
  class AngularVelocity : public Vector3D {
  public:

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    AngularVelocity() = default;

    using Vector3D::Vector3D;

    AngularVelocity(const Vector3D &rhs) : Vector3D(rhs) {}

    AngularVelocity(const Vector3DInt &rhs) :
        Vector3D(static_cast<float>(rhs.x),
                 static_cast<float>(rhs.y),
                 static_cast<float>(rhs.z)) {}

    // =========================================================================
    // -- Arithmetic operators -------------------------------------------------
    // =========================================================================

    AngularVelocity &operator+=(const AngularVelocity &rhs) {
      static_cast<Vector3D &>(*this) += static_cast<const Vector3D &>(rhs);
      return *this;
    }

    friend AngularVelocity operator+(AngularVelocity lhs, const AngularVelocity &rhs) {
      lhs += rhs;
      return lhs;
    }

    AngularVelocity &operator-=(const AngularVelocity &rhs) {
      static_cast<Vector3D &>(*this) -= static_cast<const Vector3D &>(rhs);
      return *this;
    }

    friend AngularVelocity operator-(AngularVelocity lhs, const AngularVelocity &rhs) {
      lhs -= rhs;
      return lhs;
    }

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const AngularVelocity &rhs) const {
      return static_cast<const Vector3D &>(*this) == static_cast<const Vector3D &>(rhs);
    }

    bool operator!=(const AngularVelocity &rhs) const {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- Conversions to UE4 types ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    // in future potentially transform deg/s -> rad/s?
    AngularVelocity(const FVector &vector)
      : Vector3D(vector.X, vector.Y, vector.Z) {}

    // in future potentially transform rad/s -> deg/s?
    operator FVector() const {
      return FVector{x, y, z}; 
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace geom
} // namespace carla
