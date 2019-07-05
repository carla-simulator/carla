// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/Episode.h"

#include "carla/Logging.h"
#include "carla/client/detail/Client.h"
#include "carla/sensor/Deserializer.h"

#include <exception>

namespace carla {
namespace client {
namespace detail {

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
        do {
          if (prev->GetFrame() >= next->GetFrame()) {
            self->_on_tick_callbacks.Call(next);
            return;
          }
        } while (!self->_state.compare_exchange(&prev, next));

        if (next->GetEpisodeId() != prev->GetEpisodeId()) {
          self->OnEpisodeStarted();
        }

        // Notify waiting threads and do the callbacks.
        self->_snapshot.SetValue(next);
        self->_on_tick_callbacks.Call(next);
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

  std::vector<rpc::Actor> Episode::GetActorsById(const std::vector<ActorId> &actor_ids) {
    return GetActorsById_Impl(_client, _actors, actor_ids);
  }

  std::vector<rpc::Actor> Episode::GetActors() {
    return GetActorsById_Impl(_client, _actors, GetState()->GetActorIds());
  }

  void Episode::OnEpisodeStarted() {
    _actors.Clear();
    _on_tick_callbacks.Clear();
  }

} // namespace detail
} // namespace client
} // namespace carla
