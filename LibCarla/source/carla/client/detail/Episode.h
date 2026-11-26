// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
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

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
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

    /// Whether OnEpisodeChanged has finished for the episode GetId() returns.
    bool IsEpisodeChangeHandled() const {
      return _handled_episode_id->load() == GetId();
    }

    std::shared_ptr<const EpisodeState> GetState() const {
      return _state.load();
    }

    void RegisterActor(rpc::Actor actor) {
      _actors.Insert(std::move(actor));
    }

    std::optional<rpc::Actor> GetActorById(ActorId id);

    std::vector<rpc::Actor> GetActorsById(const std::vector<ActorId> &actor_ids);

    std::vector<rpc::Actor> GetActors();

    /// The actors the server leaves out of the episode state because they
    /// cannot move (static props, traffic signs and lights) or because they
    /// are where their walker is (AI walker controllers, the followers), and
    /// the episode and static-actors version
    /// (EpisodeState::GetStaticActorsVersion) they were fetched at.
    struct StaticActors {
      uint64_t episode_id = 0u;
      uint32_t version = 0u;
      std::unordered_map<ActorId, ActorSnapshot> actors;
      /// Follower -> its walker.
      std::unordered_map<ActorId, ActorId> followers;
    };

    /// The current static actors. Fetched with one call the first time they
    /// are needed after the episode state reports a new version, never on the
    /// stream thread. GetActors() does not list them.
    std::shared_ptr<const StaticActors> GetStaticActors();

    /// The snapshot of a static actor (transform, Active state, no motion, the
    /// sign id of a traffic sign, the times and pole of a traffic light), or
    /// of a follower (the transform of its walker), or nothing. Does not
    /// throw.
    std::optional<ActorSnapshot> GetStaticActorSnapshot(ActorId id);

    std::optional<WorldSnapshot> WaitForState(time_duration timeout) {
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

    void OnEpisodeChanged(uint64_t episode_id);

    Client &_client;

    AtomicSharedPtr<const EpisodeState> _state;

    /// The last state decoded from the stream, published or not: the next delta applies on it. Stream thread only.
    std::shared_ptr<const EpisodeState> _last_received;

    std::string _pending_exceptions_msg;

    CachedActorList _actors;

    CallbackList<WorldSnapshot> _on_tick_callbacks;

    CallbackList<WorldSnapshot> _on_map_change_callbacks;

    CallbackList<WorldSnapshot> _on_light_update_callbacks;

    RecurrentSharedFuture<WorldSnapshot> _snapshot;

    AtomicSharedPtr<WalkerNavigation> _walker_navigation;

    AtomicSharedPtr<const StaticActors> _static_actors;

    std::mutex _static_mutex;

    const streaming::Token _token;

    bool _pending_exceptions = false;

    bool _should_update_map = true;

    std::weak_ptr<Simulator> _simulator;

    /// Shared with the TM reset thread, which may outlive this Episode.
    std::shared_ptr<std::atomic<uint64_t>> _handled_episode_id;
  };

} // namespace detail
} // namespace client
} // namespace carla
