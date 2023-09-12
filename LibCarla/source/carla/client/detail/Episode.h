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
#include "carla/client/WorldSnapshot.h"
#include "carla/client/detail/CachedActorList.h"
#include "carla/client/detail/CallbackList.h"
#include "carla/client/detail/EpisodeState.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/rpc/EpisodeInfo.h"

#include <vector>

namespace carla {
namespace client {
namespace detail {

  class Client;
  class WalkerNavigation;

  /// Holds the current episode, and the current episode state.
  ///
  /// The episode state changes in the background each time a world tick is
  /// received. The episode may change with any background update if the
  /// simulator has loaded a new episode.
  class Episode
    : public std::enable_shared_from_this<Episode>,
      private NonCopyable {
  public:

    explicit Episode(Client &client, std::weak_ptr<Simulator> simulator);

    ~Episode();

    void Listen();

    auto GetId() const {
      return GetState()->GetEpisodeId();
    }

    std::shared_ptr<const EpisodeState> GetState() const {
      return _state.load();
    }

    void RegisterActor(rpc::Actor actor) {
      _actors.Insert(std::move(actor));
    }

    boost::optional<rpc::Actor> GetActorById(ActorId id);

    std::vector<rpc::Actor> GetActorsById(const std::vector<ActorId> &actor_ids);

    std::vector<rpc::Actor> GetActors();

    boost::optional<WorldSnapshot> WaitForState(time_duration timeout) {
      return _snapshot.WaitFor(timeout);
    }

    size_t RegisterOnTickEvent(std::function<void(WorldSnapshot)> callback) {
      return _on_tick_callbacks.Push(std::move(callback));
    }

    void RemoveOnTickEvent(size_t id) {
      _on_tick_callbacks.Remove(id);
    }

    size_t RegisterOnMapChangeEvent(std::function<void(WorldSnapshot)> callback) {
      return _on_map_change_callbacks.Push(std::move(callback));
    }

    void RemoveOnMapChangeEvent(size_t id) {
      _on_map_change_callbacks.Remove(id);
    }

    size_t RegisterLightUpdateChangeEvent(std::function<void(WorldSnapshot)> callback) {
      return _on_light_update_callbacks.Push(std::move(callback));
    }

    void RemoveLightUpdateChangeEvent(size_t id) {
      _on_light_update_callbacks.Remove(id);
    }

    void SetPedestriansCrossFactor(float percentage);

    void SetPedestriansSeed(unsigned int seed);

    void AddPendingException(std::string e) {
      _pending_exceptions = true;
      _pending_exceptions_msg = e;
    }

    bool HasMapChangedSinceLastCall();

    std::shared_ptr<WalkerNavigation> CreateNavigationIfMissing();

  private:

    Episode(Client &client, const rpc::EpisodeInfo &info, std::weak_ptr<Simulator> simulator);

    void OnEpisodeStarted();

    void OnEpisodeChanged();

    Client &_client;

    AtomicSharedPtr<const EpisodeState> _state;

    std::string _pending_exceptions_msg;

    CachedActorList _actors;

    CallbackList<WorldSnapshot> _on_tick_callbacks;

    CallbackList<WorldSnapshot> _on_map_change_callbacks;

    CallbackList<WorldSnapshot> _on_light_update_callbacks;

    RecurrentSharedFuture<WorldSnapshot> _snapshot;

    AtomicSharedPtr<WalkerNavigation> _walker_navigation;

    const streaming::Token _token;

    bool _pending_exceptions = false;

    bool _should_update_map = true;

    std::weak_ptr<Simulator> _simulator;
  };

} // namespace detail
} // namespace client
} // namespace carla
