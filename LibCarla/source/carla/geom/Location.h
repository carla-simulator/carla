// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Math/Vector.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

  class Location : public Vector3D {
  public:

    Location() = default;

    using Vector3D::Vector3D;

    using Vector3D::x;
    using Vector3D::y;
    using Vector3D::z;

    using Vector3D::msgpack_pack;
    using Vector3D::msgpack_unpack;
    using Vector3D::msgpack_object;

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

    /// @todo Do we need to multiply locations?
    Location &operator*=(const double &rhs) {
      static_cast<Vector3D &>(*this) *= rhs;
      return *this;
    }

    friend Location operator*(Location lhs, double rhs) {
      lhs *= rhs;
      return lhs;
    }

    friend Location operator*(double lhs, Location rhs) {
      rhs *= lhs;
      return rhs;
    }

    bool operator==(const Location &rhs) const {
      return static_cast<const Vector3D &>(*this) == rhs;
    }

    bool operator!=(const Location &rhs) const {
      return !(*this == rhs);
    }

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
