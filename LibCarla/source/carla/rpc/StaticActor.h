// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/geom/Transform.h"
#include "carla/rpc/ActorId.h"

#include <string>

namespace carla {
namespace rpc {

  /// An actor the server leaves out of the per-tick episode state because it
  /// cannot move (a static prop, a traffic sign), as returned by the
  /// "get_static_actors" call: what the episode state would say about it.
  struct StaticActor {

    ActorId id = 0u;

    geom::Transform transform;

    /// OpenDRIVE id of a traffic sign; empty for any other actor.
    std::string sign_id;

    MSGPACK_DEFINE_ARRAY(id, transform, sign_id);
  };

} // namespace rpc
} // namespace carla
