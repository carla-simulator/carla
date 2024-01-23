// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/ActorVariant.h"

#include "carla/client/detail/ActorFactory.h"
#include "carla/client/ActorList.h"

namespace carla {
namespace client {
namespace detail {

  void ActorVariant::MakeActor(EpisodeProxy episode) const {
    _value = detail::ActorFactory::MakeActor(
        episode,
        std::get<rpc::Actor>(std::move(_value)),
        GarbageCollectionPolicy::Disabled);
  }

} // namespace detail
} // namespace client
} // namespace carla
