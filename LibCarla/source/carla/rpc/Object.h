// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

#include "carla/geom/BoundingBox.h"
#include "carla/rpc/Transform.h"

namespace carla {
namespace rpc {

  // Name is under discussion
  struct CarlaObject {
    Transform transform;
    geom::BoundingBox bounding_box;
    uint64_t id = 0;
    std::string name;
    //CityObjectLabel type;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    CarlaObject(const FCarlaObject &CarlaObject)
      : transform(CarlaObject.Transform),
        bounding_box(CarlaObject.BoundingBox),
        id(CarlaObject.Id),
        name(TCHAR_TO_UTF8(*CarlaObject.Name)) {}

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(transform, bounding_box, id, name);
  };

} // namespace rpc
} // namespace carla

