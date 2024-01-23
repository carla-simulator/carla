// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/client/GarbageCollectionPolicy.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/rpc/Actor.h"

namespace carla {
namespace client {

  class Actor;

namespace detail {

  class ActorFactory {
  public:

    /// Create an Actor based on the provided @a actor_description. @a episode
    /// must point to the episode in which the actor is living.
    ///
    /// Do not call this class directly, use Simulator::MakeActor.
    ///
    /// If @a garbage_collection_policy is GarbageCollectionPolicy::Enabled, the
    /// shared pointer returned is provided with a custom deleter that calls
    /// Destroy() on the actor.
    static SharedPtr<Actor> MakeActor(
        EpisodeProxy episode,
        rpc::Actor actor_description,
        GarbageCollectionPolicy garbage_collection_policy);
  };

} // namespace detail
} // namespace client
} // namespace carla
