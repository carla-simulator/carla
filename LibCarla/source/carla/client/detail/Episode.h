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

  class Client;

  /// Holds the current episode, and the current episode state.
  ///
  /// The episode state changes in the background each time a world tick is
  /// received. The episode may change with any background update if the
  /// simulator has loaded a new episode.
  class Episode
    : public std::enable_shared_from_this<Episode>,
      private NonCopyable {
  public:

    explicit Episode(uint64_t id) : _id(id) {}

    auto GetId() const {
      return GetState()->GetEpisodeId();
    }

    std::shared_ptr<const EpisodeState> GetState() const {
      return _state.load();
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

    Episode(Client &client, const rpc::EpisodeInfo &info);

    void OnEpisodeStarted();

    Client &_client;

    AtomicSharedPtr<const EpisodeState> _state;

    CachedActorList _actors;

    CallbackList<Timestamp> _on_tick_callbacks;

    RecurrentSharedFuture<Timestamp> _timestamp;

    const streaming::Token _token;
  };

} // namespace detail
} // namespace client
} // namespace carla
