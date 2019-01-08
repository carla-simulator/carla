// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/geom/Location.h"
#include "carla/geom/Rotation.h"
#include "carla/geom/Math.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include "Math/Transform.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4


namespace carla {
namespace geom {

  class Transform {
  public:

    Transform() = default;

    Transform(const Location &in_location, const Rotation &in_rotation)
      : location(in_location),
        rotation(in_rotation) {
      }

    Location location;
    Rotation rotation;

    bool operator==(const Transform &rhs) const  {
      return (location == rhs.location) && (rotation == rhs.rotation);
    }

    bool operator!=(const Transform &rhs) const  {
      return !(*this == rhs);
    }


    Location GetLocation() const {
      return location;
    }

    Rotation GetRotation() const {
      return rotation;
    }

    template<class T>
    inline void TransformPointList(std::vector<T> &in_point_list) const {
      for (T in_point : in_point_list) {
        TransformPoint(in_point);
      }
    }

    inline void TransformPoint (Location &in_point) const {

      // Rotate
      double cy = cos(Math::to_radians(rotation.yaw));
      double sy = sin(Math::to_radians(rotation.yaw));
      double cr = cos(Math::to_radians(rotation.roll));
      double sr = sin(Math::to_radians(rotation.roll));
      double cp = cos(Math::to_radians(rotation.pitch));
      double sp = sin(Math::to_radians(rotation.pitch));

      Location out_point;
      out_point.x = in_point.x * ( cp * cy )
                  + in_point.y * ( cy * sp * sr - sy * cr)
                  + in_point.z * (-cy * sp * cr - sy * sr);
      out_point.y = in_point.x * ( cp * sy)
                  + in_point.y * ( sy * sp * sr + cy * cr)
                  + in_point.z * (-sy * sp * cr + cy * sr);

      out_point.z = in_point.x *  (sp)
                  + in_point.y * -(cp * sr)
                  + in_point.z *  (cp * cr);

      // Translate
      out_point += location;

      in_point = out_point;
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Transform(const FTransform &transform)
      : Transform(Location(transform.GetLocation()), Rotation(transform.Rotator())) {}

    operator FTransform() const {
      const FVector scale{1.0f, 1.0f, 1.0f};
      return FTransform{FRotator(rotation), FVector(location), scale};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(location, rotation);
  };

} // namespace geom
} // namespace carla
