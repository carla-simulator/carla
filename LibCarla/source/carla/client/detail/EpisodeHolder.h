// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicSharedPtr.h"
#include "carla/NonCopyable.h"
#include "carla/RecurrentSharedFuture.h"
#include "carla/client/Timestamp.h"
#include "carla/client/detail/CallbackList.h"
#include "carla/client/detail/Episode.h"
#include "carla/client/detail/EpisodeState.h"

namespace carla {
namespace client {
namespace detail {

  class Client;

  /// Holds the current episode, and the current episode state.
  ///
  /// The episode state changes in the background each time a world tick is
  /// received. The episode may change with any background update if the
  /// simulator has loaded a new episode.
  class EpisodeHolder
    : public std::enable_shared_from_this<EpisodeHolder>,
      private NonCopyable {
  public:

    explicit EpisodeHolder(Client &client);

    ~EpisodeHolder();

    void Listen();

    std::shared_ptr<Episode> GetEpisode() {
      return _episode.load();
    }

    std::shared_ptr<const EpisodeState> GetState() const {
      return _state.load();
    }

    void RegisterActor(rpc::Actor actor) {
      GetEpisode()->RegisterActor(std::move(actor));
    }

    std::vector<rpc::Actor> GetActors();

    boost::optional<Timestamp> WaitForState(time_duration timeout) {
      return _timestamp.WaitFor(timeout);
    }

    void RegisterOnTickEvent(std::function<void(Timestamp)> callback) {
      _on_tick_callbacks.RegisterCallback(std::move(callback));
    }

  private:

    EpisodeHolder(Client &client, const rpc::EpisodeInfo &info);

    Client &_client;

    AtomicSharedPtr<Episode> _episode;

    AtomicSharedPtr<const EpisodeState> _state;

    RecurrentSharedFuture<Timestamp> _timestamp;

    CallbackList<Timestamp> _on_tick_callbacks;

    const streaming::Token _token;
  };

} // namespace detail
} // namespace client
} // namespace carla
