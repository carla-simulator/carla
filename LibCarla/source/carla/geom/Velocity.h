// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"
#include "carla/geom/Vector3DInt.h"
#include "carla/geom/Math.h"
#include "carla/geom/Quaternion.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Math/Vector.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

  class Velocity : public Vector3D {
  public:

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    Velocity() = default;

    using Vector3D::Vector3D;

    Velocity(const Vector3D &rhs) : Vector3D(rhs) {}

    Velocity(const Vector3DInt &rhs) :
        Vector3D(static_cast<float>(rhs.x),
                 static_cast<float>(rhs.y),
                 static_cast<float>(rhs.z)) {}

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    // return the signed speed of the velocity vector in reference to the provided orientation of the respective object
    auto Speed(carla::geom::Quaternion const &orientation) const {
      bool signbit_cosine = false;
      auto const speed = Length();
      if ( speed > 1e-2 ) {
        // decide if we are driving forward or backward by the cosine between forward vector and velocity vector
        auto const forward_unit_vector = orientation.GetForwardVector();
        signbit_cosine = carla::geom::Math::SignBitCosineAngleFromUnitVector(*this, forward_unit_vector);
      }
      if ( signbit_cosine ) {
        return -speed;
      }
      else {
        return speed;
      }
    }

    // =========================================================================
    // -- Arithmetic operators -------------------------------------------------
    // =========================================================================

    Velocity &operator+=(const Velocity &rhs) {
      static_cast<Vector3D &>(*this) += static_cast<const Vector3D &>(rhs);
      return *this;
    }

    friend Velocity operator+(Velocity lhs, const Velocity &rhs) {
      lhs += rhs;
      return lhs;
    }

    Velocity &operator-=(const Velocity &rhs) {
      static_cast<Vector3D &>(*this) -= static_cast<const Vector3D &>(rhs);
      return *this;
    }

    friend Velocity operator-(Velocity lhs, const Velocity &rhs) {
      lhs -= rhs;
      return lhs;
    }

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const Velocity &rhs) const {
      return static_cast<const Vector3D &>(*this) == static_cast<const Vector3D &>(rhs);
    }

    bool operator!=(const Velocity &rhs) const {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- Conversions to UE4 types ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    // from cm/s to m/s.
    Velocity(const FVector &vector)
      : Velocity(1e-2f * vector.X, 1e-2f * vector.Y, 1e-2f * vector.Z) {}

    // from m/s to cm/s
    operator FVector() const {
      return FVector{1e2f * x, 1e2f * y, 1e2f * z}; 
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

  inline std::ostream &operator<<(std::ostream &out, const Velocity &vector3D) {
    carla::geom::WriteVector3D(out, "Velocity", vector3D);
    return out;
  }

} // namespace geom
} // namespace carla
