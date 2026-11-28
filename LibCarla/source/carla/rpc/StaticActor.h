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

  /// An actor the server leaves out of the per-tick episode state, as
  /// returned by the "get_static_actors" call: what the episode state would
  /// say about it. Either it cannot move (a static prop, a traffic sign or
  /// light), or it is an AI walker controller that is where its walker is.
  struct StaticActor {

    ActorId id = 0u;

    geom::Transform transform;

    /// OpenDRIVE id of a traffic sign or light; empty for any other actor.
    std::string sign_id;

    /// For an AI walker controller, the walker whose transform it has; 0 for
    /// any other actor.
    ActorId parent_id = 0u;

    /// What does not change on every tick of a traffic light; its state,
    /// elapsed time and frozen flag are in the episode state.
    float green_time = 0.0f;
    float yellow_time = 0.0f;
    float red_time = 0.0f;
    uint32_t pole_index = 0u;

    MSGPACK_DEFINE_ARRAY(id, transform, sign_id, parent_id, green_time, yellow_time, red_time, pole_index);
  };

} // namespace rpc
} // namespace carla
