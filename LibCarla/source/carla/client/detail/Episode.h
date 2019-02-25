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
#include "carla/client/detail/CachedActorList.h"
#include "carla/client/detail/CallbackList.h"
#include "carla/client/detail/EpisodeState.h"
#include "carla/rpc/EpisodeInfo.h"

namespace carla {
namespace client {
namespace detail {

  class Client;

  /// Represents the episode running on the Simulator.
  class Episode
    : public std::enable_shared_from_this<Episode>,
      private NonCopyable {
  public:

    explicit Episode(Client &client);

    ~Episode();

    void Listen();

    auto GetId() const {
      return _description.id;
    }

    const std::string &GetMapName() const {
      return _description.map_name;
    }

    std::shared_ptr<const EpisodeState> GetState() const {
      auto state = _state.load();
      DEBUG_ASSERT(state != nullptr);
      return state;
    }

    void RegisterActor(rpc::Actor actor) {
      _actors.Insert(std::move(actor));
    }

    std::vector<rpc::Actor> GetActors();

    boost::optional<Timestamp> WaitForState(time_duration timeout) {
      return _timestamp.WaitFor(timeout);
    }

    void RegisterOnTickEvent(std::function<void(Timestamp)> callback) {
      _on_tick_callbacks.RegisterCallback(std::move(callback));
    }

  private:

    Client &_client;

    const rpc::EpisodeInfo _description;

    RecurrentSharedFuture<Timestamp> _timestamp;

    AtomicSharedPtr<const EpisodeState> _state;

    CachedActorList _actors;

    CallbackList<Timestamp> _on_tick_callbacks;
  };

} // namespace detail
} // namespace client
} // namespace carla
