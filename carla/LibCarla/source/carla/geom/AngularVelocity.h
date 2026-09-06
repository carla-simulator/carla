// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"

namespace carla {
namespace geom {

  /// Angular velocity in degrees per second. Inherits `Vector3D` storage and
  /// arithmetic and carries unit semantics so consumers do not silently mix
  /// `deg/s` and `rad/s` payloads at API boundaries.
  struct AngularVelocity : Vector3D {

    AngularVelocity() = default;

    constexpr AngularVelocity(float ix, float iy, float iz)
      : Vector3D(ix, iy, iz) {
    }

    explicit constexpr AngularVelocity(const Vector3D &v)
      : Vector3D(v) {
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    /// Build an `AngularVelocity` (deg/s) from an Unreal `FVector` already
    /// expressed in degrees per second.
    explicit AngularVelocity(const FVector &v_deg_per_s)
      : Vector3D(
            static_cast<float>(v_deg_per_s.X),
            static_cast<float>(v_deg_per_s.Y),
            static_cast<float>(v_deg_per_s.Z)) {
    }

    /// Return this `AngularVelocity` as an Unreal `FVector` in degrees per
    /// second.
    FVector ToFVectorDegPerSecond() const {
      return FVector{x, y, z};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace geom
} // namespace carla
