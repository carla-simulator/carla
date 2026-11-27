// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/Episode.h"

#include "carla/Logging.h"
#include "carla/client/detail/Client.h"
#include "carla/client/detail/WalkerNavigation.h"
#include "carla/sensor/Deserializer.h"
#include "carla/trafficmanager/TrafficManager.h"

#include <algorithm>
#include <cstring>
#include <exception>
#include <utility>

class FPoseSnapShot;

namespace carla {
namespace client {
namespace detail {

using namespace std::chrono_literals;

  static auto &CastData(const sensor::SensorData &data) {
    using target_t = const sensor::data::RawEpisodeState;
    return static_cast<target_t &>(data);
  }

  template <typename RangeT>
  static auto GetActorsById_Impl(Client &client, CachedActorList &actors, const RangeT &actor_ids) {
    auto missing_ids = actors.GetMissingIds(actor_ids);
    if (!missing_ids.empty()) {
      actors.InsertRange(client.GetActorsById(missing_ids));
    }
    return actors.GetActorsById(actor_ids);
  }

  Episode::Episode(Client &client, std::weak_ptr<Simulator> simulator)
    : Episode(client, client.GetEpisodeInfo(), simulator) {}

  Episode::Episode(Client &client, const rpc::EpisodeInfo &info, std::weak_ptr<Simulator> simulator)
    : _client(client),
      _state(std::make_shared<EpisodeState>(info.id)),
      _token(info.token),
      _simulator(simulator),
      _handled_episode_id(std::make_shared<std::atomic<uint64_t>>(info.id)) {}

  Episode::~Episode() {
    try {
      _client.UnSubscribeFromStream(_token);
    } catch (const std::exception &e) {
      log_error("exception trying to disconnect from episode:", e.what());
    }
  }

  void Episode::Listen() {
    std::weak_ptr<Episode> weak = shared_from_this();
    _client.SubscribeToStream(_token, [weak](auto buffer) {
      auto self = weak.lock();
      if (self != nullptr) {

        auto data = sensor::Deserializer::Deserialize(std::move(buffer));
        auto next = EpisodeState::Make(CastData(*data), self->_last_received);
        if (next == nullptr) {
          // Unreachable while the server sends a whole state to a session that missed one; logged once per break.
          if (std::exchange(self->_last_received, nullptr) != nullptr) {
            log_error("episode state: a delta does not follow the last state received, waiting for a whole one");
          }
          return;
        }
        self->_last_received = next;
        auto prev = self->GetState();

        // TODO: Update how the map change is detected
        bool HasMapChanged = next->HasMapChanged();
        bool UpdateLights = next->IsLightUpdatePending();

        /// Check for pending exceptions (Mainly TM server closed)
        if(self->_pending_exceptions) {

          /// Mark pending exception false
          self->_pending_exceptions = false;

          /// Create exception for the error message
          auto exception(self->_pending_exceptions_msg);
          // Notify waiting threads that exception occurred
          self->_snapshot.SetException(std::runtime_error(exception));
        }
        /// Sensor case: inconsistent data
        else {
          bool episode_changed = (next->GetEpisodeId() != prev->GetEpisodeId());

          if (episode_changed) {
            // Drop the previous episode's WalkerNavigation BEFORE the new
            // episode id becomes visible (the compare_exchange below is what
            // unblocks load_world): it caches that map's navmesh (.bin) and
            // its server-side-navigation probe, so any query through it --
            // e.g. get_random_location_from_navigation -- would keep
            // answering with the previous map's data after a load_world.
            // OnEpisodeStarted() used to do this but is no longer called on
            // episode change (see OnEpisodeChanged below).
            self->_walker_navigation.reset();
            // Same ordering rule for the map cache: load_world returns as
            // soon as the new episode id is published below, and the first
            // world.get_map() must already see the cache as stale.
            self->_should_update_map = true;
          }

          do {
            if (prev->GetFrame() >= next->GetFrame() && !episode_changed) {
              self->_on_tick_callbacks.Call(next);
              return;
            }
          } while (!self->_state.compare_exchange(&prev, next));

          if(UpdateLights || HasMapChanged) {
            // The LightManager queries the server here; a time-out or a dying Simulator must not terminate the process.
            try {
              self->_on_light_update_callbacks.Call(next);
            } catch (const std::exception &e) {
              log_error("failed to update the lights for the new snapshot:", e.what());
            }
          }

          if(HasMapChanged) {
            self->_should_update_map = true;
          }

          /// Episode change
          if(episode_changed) {
            self->OnEpisodeChanged(next->GetEpisodeId());
          }

          // Notify waiting threads and do the callbacks.
          self->_snapshot.SetValue(next);

          // Call user callbacks.
          self->_on_tick_callbacks.Call(next);
        }
      }
    });
  }

  std::optional<rpc::Actor> Episode::GetActorById(ActorId id) {
    auto actor = _actors.GetActorById(id);
    if (!actor.has_value()) {
      auto actor_list = _client.GetActorsById({id});
      if (!actor_list.empty()) {
        actor = std::move(actor_list.front());
        _actors.Insert(*actor);
      }
    }
    return actor;
  }

  std::vector<rpc::Actor> Episode::GetActorsById(const std::vector<ActorId> &actor_ids) {
    return GetActorsById_Impl(_client, _actors, actor_ids);
  }

  std::vector<rpc::Actor> Episode::GetActors() {
    return GetActorsById_Impl(_client, _actors, GetState()->GetActorIds());
  }

  std::shared_ptr<const Episode::StaticActors> Episode::GetStaticActors() {
    auto state = GetState();
    auto cached = _static_actors.load();
    const auto is_current = [&]() {
      return cached != nullptr &&
          cached->episode_id == state->GetEpisodeId() &&
          cached->version == state->GetStaticActorsVersion();
    };
    if (is_current()) {
      return cached;
    }
    std::lock_guard<std::mutex> lock(_static_mutex);
    // Another reader may have fetched them while this one waited for the lock.
    state = GetState();
    cached = _static_actors.load();
    if (is_current()) {
      return cached;
    }
    auto fresh = std::make_shared<StaticActors>();
    fresh->episode_id = state->GetEpisodeId();
    fresh->version = state->GetStaticActorsVersion();
    // Version 0: no state has arrived yet, so there is nothing to ask for. The
    // version is read before the call: a change the server makes while the
    // call is in flight arrives as a newer version, and the next reader
    // refetches.
    if (fresh->version != 0u) {
      auto list = _client.GetStaticActors();
      fresh->actors.reserve(list.size());
      for (auto &&actor : list) {
        ActorSnapshot snapshot{};
        snapshot.id = actor.id;
        snapshot.actor_state = rpc::ActorState::Active;
        snapshot.transform = actor.transform;
        // Where TrafficSign::GetSignId and the TrafficLight getters read them.
        auto &light = snapshot.state.traffic_light_data;
        std::memcpy(light.sign_id, actor.sign_id.data(), std::min(sizeof(light.sign_id), actor.sign_id.size()));
        light.green_time = actor.green_time;
        light.yellow_time = actor.yellow_time;
        light.red_time = actor.red_time;
        light.pole_index = actor.pole_index;
        fresh->actors.emplace(actor.id, snapshot);
        if (actor.parent_id != 0u) {
          fresh->followers.emplace(actor.id, actor.parent_id);
        }
      }
    }
    _static_actors.store(fresh);
    return fresh;
  }

  std::optional<ActorSnapshot> Episode::GetStaticActorSnapshot(ActorId id) {
#ifndef LIBCARLA_NO_EXCEPTIONS
    try {
#endif // LIBCARLA_NO_EXCEPTIONS
      const auto statics = GetStaticActors();
      const auto it = statics->actors.find(id);
      if (it != statics->actors.end()) {
        auto snapshot = it->second;
        // A follower is where its walker is now.
        const auto follower = statics->followers.find(id);
        if (follower != statics->followers.end()) {
          const auto walker = GetState()->GetActorSnapshotIfPresent(follower->second);
          if (walker.has_value()) {
            snapshot.transform = walker->transform;
          }
        }
        return snapshot;
      }
#ifndef LIBCARLA_NO_EXCEPTIONS
    } catch (const std::exception &e) {
      // Actor::IsAlive() gets here, and destructors call it: a server that
      // does not answer means "not a static actor", never an exception.
      log_debug("failed to get the static actors:", e.what());
    }
#endif // LIBCARLA_NO_EXCEPTIONS
    return std::nullopt;
  }

  void Episode::OnEpisodeStarted() {
    _actors.Clear();
    _on_tick_callbacks.Clear();
    _walker_navigation.reset();
    traffic_manager::TrafficManager::Release();
  }

  void Episode::OnEpisodeChanged(uint64_t episode_id) {
    // Hand the reset to the process-wide TM thread: resetting here can destroy clients and deadlock.
    traffic_manager::TrafficManager::RequestReset(episode_id,
        [handled = _handled_episode_id, episode_id]() { handled->store(episode_id); });
  }

  bool Episode::HasMapChangedSinceLastCall() {
    if(_should_update_map) {
      _should_update_map = false;
      return true;
    }
    return false;
  }

  std::shared_ptr<WalkerNavigation> Episode::CreateNavigationIfMissing() {
    std::shared_ptr<WalkerNavigation> nav;
    do {
      nav = _walker_navigation.load();
      if (nav == nullptr) {
        // Queried once per episode: _walker_navigation is reset on episode
        // change (see Listen), so a load_world onto/off a server-side-nav
        // map re-detects.
        auto new_nav = std::make_shared<WalkerNavigation>(
            _simulator, _client.IsNavigationServerSide());
        _walker_navigation.compare_exchange(&nav, new_nav);
      }
    } while (nav == nullptr);
    return nav;
  }

} // namespace detail
} // namespace client
} // namespace carla
