// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/geom/Transform.h"

#include <string>
#include <vector>

namespace carla {
namespace rpc {

  class MapInfo {
  public:

    std::string name;

    std::string open_drive_file;

    std::vector<geom::Transform> recommended_spawn_points;

    MSGPACK_DEFINE_ARRAY(name, open_drive_file, recommended_spawn_points);
  };

} // namespace rpc
} // namespace carla
