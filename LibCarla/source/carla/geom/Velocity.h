// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"

namespace carla {
namespace geom {

  /// Linear velocity in metres per second. Inherits the storage and arithmetic
  /// of `Vector3D` while carrying unit semantics so consumers do not
  /// accidentally mix `cm/s` and `m/s` payloads at API boundaries.
  struct Velocity : Vector3D {

    Velocity() = default;

    constexpr Velocity(float ix, float iy, float iz)
      : Vector3D(ix, iy, iz) {
    }

    explicit constexpr Velocity(const Vector3D &v)
      : Vector3D(v) {
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    /// Build a `Velocity` (m/s) from an Unreal `FVector` expressed in
    /// centimetres per second.
    explicit Velocity(const FVector &v_cm_per_s)
      : Vector3D(
            static_cast<float>(v_cm_per_s.X) * 1e-2f,
            static_cast<float>(v_cm_per_s.Y) * 1e-2f,
            static_cast<float>(v_cm_per_s.Z) * 1e-2f) {
    }

    /// Return this `Velocity` as an Unreal `FVector` in centimetres per
    /// second.
    FVector ToFVectorCmPerSecond() const {
      return FVector{x * 1e2f, y * 1e2f, z * 1e2f};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace geom
} // namespace carla
