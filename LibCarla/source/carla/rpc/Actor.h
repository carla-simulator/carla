// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/rpc/ActorDescription.h"

namespace carla {
namespace rpc {

  class Actor {
  public:

    using id_type = uint32_t;

    Actor() = default;

    id_type id;

    ActorDescription description;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Actor(FActorView View)
      : id(View.GetActorId()),
        description(*View.GetActorDescription()) {
      DEBUG_ASSERT(View.IsValid());
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(id, description);
  };

} // namespace rpc
} // namespace carla
