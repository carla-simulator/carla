// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/geom/Transform.h"
#include "carla/rpc/ActorId.h"

namespace carla {
namespace rpc {

  /// An actor the server leaves out of the per-tick episode state when it runs
  /// with -carla-omit-static-actors (a static mesh that cannot move): its id
  /// and transform, as returned by the "get_static_actors" call.
  struct StaticActor {

    ActorId id = 0u;

    geom::Transform transform;

    MSGPACK_DEFINE_ARRAY(id, transform);
  };

} // namespace rpc
} // namespace carla
