// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/ActorBlueprint.h"

namespace carla {
namespace rpc {

  class Actor {
  public:

    using id_type = uint32_t;

    id_type id;

    ActorBlueprint blueprint;

    MSGPACK_DEFINE_ARRAY(id, blueprint);
  };

} // namespace rpc
} // namespace carla
