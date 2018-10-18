// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/rpc/Actor.h"

#include <boost/iterator/transform_iterator.hpp>
#include <boost/variant.hpp>

#include <vector>

namespace carla {
namespace client {

  class Actor;

  class ActorList {
  private:

    template <typename It>
    auto MakeIterator(It it) const {
      return boost::make_transform_iterator(it, [this](auto &v) {
        return RetrieveActor(v);
      });
    }

  public:

    SharedPtr<Actor> operator[](size_t pos) const {
      return RetrieveActor(_actors[pos]);
    }

    SharedPtr<Actor> at(size_t pos) const {
      return RetrieveActor(_actors.at(pos));
    }

    auto begin() const {
      return MakeIterator(_actors.begin());
    }

    auto end() const {
      return MakeIterator(_actors.end());
    }

    bool empty() const {
      return _actors.empty();
    }

    size_t size() const {
      return _actors.size();
    }

  private:

    friend class World;

    ActorList(detail::EpisodeProxy episode, std::vector<rpc::Actor> actors);

    using value_type = boost::variant<rpc::Actor, SharedPtr<Actor>>;

    SharedPtr<Actor> RetrieveActor(value_type &value) const;

    detail::EpisodeProxy _episode;

    mutable std::vector<value_type> _actors;
  };

} // namespace client
} // namespace carla
