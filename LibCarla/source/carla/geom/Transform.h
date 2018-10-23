// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/geom/Location.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Transform.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

  class Rotation {
  public:

    Rotation() = default;

    Rotation(float p, float y, float r)
      : pitch(p),
        yaw(y),
        roll(r) {}

    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 0.0f;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Rotation(const FRotator &rotator)
      : Rotation(rotator.Pitch, rotator.Yaw, rotator.Roll) {}

    operator FRotator() const {
      return FRotator{pitch, yaw, roll};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(pitch, yaw, roll);
  };

  class Transform {
  public:

    Transform() = default;

    Transform(const Location &in_location, const Rotation &in_rotation)
      : location(in_location),
        rotation(in_rotation) {}

    Location location;
    Rotation rotation;

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
