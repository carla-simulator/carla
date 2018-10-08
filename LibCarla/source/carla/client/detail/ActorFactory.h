// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/client/GarbageCollectionPolicy.h"
#include "carla/client/detail/Episode.h"
#include "carla/rpc/Actor.h"

namespace carla {
namespace client {

  class Actor;

namespace detail {

  class ActorFactory {
  public:

    static SharedPtr<Actor> MakeActor(
        Episode episode,
        rpc::Actor actor_description,
        GarbageCollectionPolicy gc = GarbageCollectionPolicy::Inherit);
  };

} // namespace detail
} // namespace client
} // namespace carla
