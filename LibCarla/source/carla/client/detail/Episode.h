// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/client/detail/CachedActorList.h"
#include "carla/client/detail/Client.h"
#include "carla/rpc/EpisodeInfo.h"

namespace carla {
namespace client {
namespace detail {

  /// Represents the episode running on the Simulator.
  class Episode
    : public std::enable_shared_from_this<Episode>,
      private NonCopyable {
  public:

    explicit Episode(uint64_t id) : _id(id) {}

    auto GetId() const {
      return _id;
    }

    void RegisterActor(rpc::Actor actor) {
      _actors.Insert(std::move(actor));
    }

    template <typename RangeT>
    std::vector<rpc::Actor> GetActors(Client &client, const RangeT &actor_ids) {
      auto missing_ids = _actors.GetMissingIds(actor_ids);
      if (!missing_ids.empty()) {
        _actors.InsertRange(client.GetActorsById(missing_ids));
      }
      return _actors.GetActorsById(actor_ids);
    }

  private:

    uint64_t _id;

    CachedActorList _actors;
  };

} // namespace detail
} // namespace client
} // namespace carla
