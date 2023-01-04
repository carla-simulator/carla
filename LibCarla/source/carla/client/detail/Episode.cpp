// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
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

#include <exception>

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

  Episode::Episode(Client &client)
    : Episode(client, client.GetEpisodeInfo()) {}

  Episode::Episode(Client &client, const rpc::EpisodeInfo &info)
    : _client(client),
      _state(std::make_shared<EpisodeState>(info.id)),
      _token(info.token) {}

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
        auto next = std::make_shared<const EpisodeState>(CastData(*data));
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

          do {
            if (prev->GetFrame() >= next->GetFrame() && !episode_changed) {
              self->_on_tick_callbacks.Call(next);
              return;
            }
          } while (!self->_state.compare_exchange(&prev, next));

          if(UpdateLights || HasMapChanged) {
            self->_on_light_update_callbacks.Call(next);
          }

          if(HasMapChanged) {
            self->_should_update_map = true;
          }

          /// Episode change
          if(episode_changed) {
            self->OnEpisodeChanged();
          }

          // Notify waiting threads and do the callbacks.
          self->_snapshot.SetValue(next);

          // Call user callbacks.
          self->_on_tick_callbacks.Call(next);
        }
      }
    });
  }

  boost::optional<rpc::Actor> Episode::GetActorById(ActorId id) {
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

  std::shared_ptr<WalkerNavigation> Episode::CreateNavigationIfMissing() {
    std::shared_ptr<WalkerNavigation> navigation;
    do {
      navigation = _navigation.load();
      if (navigation == nullptr) {
        auto new_navigation = std::make_shared<WalkerNavigation>(_client);
        _navigation.compare_exchange(&navigation, new_navigation);
      }
    } while (navigation == nullptr);
    return navigation;
  }

  std::vector<rpc::Actor> Episode::GetActorsById(const std::vector<ActorId> &actor_ids) {
    return GetActorsById_Impl(_client, _actors, actor_ids);
  }

  std::vector<rpc::Actor> Episode::GetActors() {
    return GetActorsById_Impl(_client, _actors, GetState()->GetActorIds());
  }

  void Episode::OnEpisodeStarted() {
    _actors.Clear();
    _on_tick_callbacks.Clear();
    _navigation.reset();
    traffic_manager::TrafficManager::Release();
  }

  void Episode::OnEpisodeChanged() {
    traffic_manager::TrafficManager::Reset();
  }

  bool Episode::HasMapChangedSinceLastCall() {
    if(_should_update_map) {
      _should_update_map = false;
      return true;
    }
    return false;
  }

  void Episode::NavigationTick() {
    // tick pedestrian navigation
    auto navigation = _navigation.load();
    if (navigation != nullptr) {
      navigation->Tick(shared_from_this());
    }
  }

} // namespace detail
} // namespace client
} // namespace carla
