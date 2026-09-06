// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"

namespace carla {
namespace geom {

  /// Internal adapter that holds a `Vector3D` in a right-handed coordinate
  /// frame. CARLA / Unreal expose vectors in a left-handed frame; `Quaternion`
  /// and `Rotation` work in right-handed math internally. The Y axis is
  /// negated on construction and re-negated on conversion back, so the
  /// handedness flip happens exactly once at the boundary.
  struct RightHandedVector3D {

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    RightHandedVector3D() = default;

    constexpr RightHandedVector3D(float ix, float iy, float iz)
      : x(ix),
        y(iy),
        z(iz) {
    }

    /// Implicit boundary conversion from a left-handed `Vector3D`. Negates
    /// the Y component so internal math runs right-handed.
    constexpr RightHandedVector3D(const Vector3D &v)
      : x(v.x),
        y(-v.y),
        z(v.z) {
    }

    /// Implicit boundary conversion back to a left-handed `Vector3D`.
    constexpr operator Vector3D() const {
      return Vector3D(x, -y, z);
    }
  };

} // namespace geom
} // namespace carla
