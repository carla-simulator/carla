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
          if (prev->GetFrameCount() >= next->GetFrameCount()) {
            self->_on_tick_callbacks.Call(next->GetTimestamp());
            return;
          }
        } while (!self->_state.compare_exchange(&prev, next));

        if (next->GetEpisodeId() != prev->GetEpisodeId()) {
          self->OnEpisodeStarted();
        }

        // Notify waiting threads and do the callbacks.
        self->_timestamp.SetValue(next->GetTimestamp());
        self->_on_tick_callbacks.Call(next->GetTimestamp());
      }
    });
  }

  std::vector<rpc::Actor> Episode::GetActors() {
    const auto state = GetState();
    const auto actor_ids = state->GetActorIds();
    auto missing_ids = _actors.GetMissingIds(actor_ids);
    if (!missing_ids.empty()) {
      _actors.InsertRange(_client.GetActorsById(missing_ids));
    }
    return _actors.GetActorsById(actor_ids);
  }

  void Episode::OnEpisodeStarted() {
    _actors.Clear();
    _on_tick_callbacks.Clear();
  }

} // namespace detail
} // namespace client
} // namespace carla
