// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ActorList.h"

#include "carla/StringUtil.h"
#include "carla/client/Actor.h"
#include "carla/client/detail/ActorFactory.h"

#include <iterator>

namespace carla {
namespace client {

  struct GetTypeIdVisitor {
    const std::string &operator()(const rpc::Actor &actor) const {
      return actor.description.id;
    }
    const std::string &operator()(const SharedPtr<Actor> &actor) const {
      return actor->GetTypeId();
    }
  };

  ActorList::ActorList(
      detail::EpisodeProxy episode,
      std::vector<rpc::Actor> actors)
    : _episode(std::move(episode)),
      _actors(std::make_move_iterator(actors.begin()), std::make_move_iterator(actors.end())) {}

  ActorList ActorList::Filter(const std::string &wildcard_pattern) const {
    ActorList filtered{_episode, {}};
    for (auto &&item : _actors) {
      const auto &id = boost::apply_visitor(GetTypeIdVisitor(), item);
      if (StringUtil::Match(id, wildcard_pattern)) {
        filtered._actors.push_back(item);
      }
    }
    return filtered;
  }

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
