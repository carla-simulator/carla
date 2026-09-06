// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"

namespace carla {
namespace geom {

  /// Linear acceleration in metres per second squared. Inherits `Vector3D`
  /// storage and arithmetic and carries unit semantics so consumers do not
  /// silently mix `cm/s^2` and `m/s^2` payloads at API boundaries.
  struct Acceleration : Vector3D {

    Acceleration() = default;

    constexpr Acceleration(float ix, float iy, float iz)
      : Vector3D(ix, iy, iz) {
    }

    explicit constexpr Acceleration(const Vector3D &v)
      : Vector3D(v) {
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    /// Build an `Acceleration` (m/s^2) from an Unreal `FVector` expressed in
    /// centimetres per second squared.
    explicit Acceleration(const FVector &a_cm_per_s2)
      : Vector3D(
            static_cast<float>(a_cm_per_s2.X) * 1e-2f,
            static_cast<float>(a_cm_per_s2.Y) * 1e-2f,
            static_cast<float>(a_cm_per_s2.Z) * 1e-2f) {
    }

    /// Return this `Acceleration` as an Unreal `FVector` in centimetres per
    /// second squared.
    FVector ToFVectorCmPerSecondSquared() const {
      return FVector{x * 1e2f, y * 1e2f, z * 1e2f};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace geom
} // namespace carla
