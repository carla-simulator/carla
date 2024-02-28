// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"

namespace carla {
namespace geom {

  class Quaternion;
  class Rotation;
  Quaternion operator*(const Quaternion& q, const Vector3D& w);
  Quaternion operator*(const Vector3D& w, const Quaternion& q);

  /** Stores a Vector3D in right handed representation required for the internal calculations */
  class RightHandedVector3D {
    public:
        RightHandedVector3D(Vector3D const& ue_vector)
        : x(ue_vector.x)
        , y(-ue_vector.y)
        , z(ue_vector.z) 
        {}

        operator Vector3D() const {
        return Vector3D(x, -y, z);
        }

    private:
      // only the implicit conversion's are accessible by the public
      RightHandedVector3D() = default;
      RightHandedVector3D(RightHandedVector3D const &) = default;
      RightHandedVector3D(RightHandedVector3D &&) = default;
      RightHandedVector3D& operator=(RightHandedVector3D const &) = default;
      RightHandedVector3D& operator=(RightHandedVector3D &&) = default;
      
      friend class Quaternion;
      friend class Rotation;
      friend Quaternion operator*(const Quaternion&, const RightHandedVector3D&);
      friend Quaternion operator*(const RightHandedVector3D&, const Quaternion&);


      float x = 0.0f;

      float y = 0.0f;

      float z = 0.0f;
  };

} // namespace geom
} // namespace carla
