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
#include "carla/rpc/ObjectLabel.h"

namespace carla {
namespace rpc {

  // Name is under discussion
  struct Mesh {
    Transform transform;
    geom::BoundingBox bounding_box;
    uint64_t id = 0;
    std::string name;
    CityObjectLabel type = CityObjectLabel::None;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Mesh(const FCarlaMesh &CarlaMesh)
      : transform(CarlaMesh.Transform),
        bounding_box(CarlaMesh.BoundingBox),
        id(CarlaMesh.Id),
        name(TCHAR_TO_UTF8(*CarlaMesh.Name)) {}

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(transform, bounding_box, id, name);
  };

} // namespace rpc
} // namespace carla

