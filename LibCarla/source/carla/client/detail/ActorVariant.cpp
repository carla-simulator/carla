// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/ActorVariant.h"

#include "carla/client/detail/ActorFactory.h"

namespace carla {
namespace client {
namespace detail {

  void ActorVariant::MakeActor(EpisodeProxy episode) const {
    _value = detail::ActorFactory::MakeActor(
        episode,
        boost::get<rpc::Actor>(std::move(_value)),
        nullptr, /// @todo We need to create the parent too.
        GarbageCollectionPolicy::Disabled);
  }

} // namespace detail
} // namespace client
} // namespace carla
