// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ActorList.h"

#include "carla/client/Actor.h"
#include "carla/client/detail/ActorFactory.h"

#include <iterator>

namespace carla {
namespace client {

  ActorList::ActorList(
      detail::EpisodeProxy episode,
      std::vector<rpc::Actor> actors)
    : _episode(std::move(episode)),
      _actors(std::make_move_iterator(actors.begin()), std::make_move_iterator(actors.end())) {}

  SharedPtr<Actor> ActorList::RetrieveActor(value_type &value) const {
    if (value.which() == 0u) {
      value = detail::ActorFactory::MakeActor(
          _episode,
          boost::get<rpc::Actor>(std::move(value)),
          GarbageCollectionPolicy::Disabled);
    }
    DEBUG_ASSERT(value.which() == 1u);
    return boost::get<SharedPtr<Actor>>(value);
  }

} // namespace client
} // namespace carla
