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
    DEBUG_ASSERT(dynamic_cast<target_t *>(&data) != nullptr);
    return static_cast<target_t &>(data);
  }

  Episode::Episode(Client &client)
    : _client(client),
      _description(client.GetEpisodeInfo()),
      _state(std::make_shared<EpisodeState>()) {}

  Episode::~Episode() {
    try {
      _client.UnSubscribeFromStream(_description.token);
    } catch (const std::exception &e) {
      log_error("exception trying to disconnect from episode:", e.what());
    }
  }

  void Episode::Listen() {
    std::weak_ptr<Episode> weak = shared_from_this();
    _client.SubscribeToStream(_description.token, [weak](auto buffer) {
      auto self = weak.lock();
      if (self != nullptr) {
        auto data = sensor::Deserializer::Deserialize(std::move(buffer));
        auto prev = self->_state.load();
        auto next = prev->DeriveNextStep(CastData(*data));
        /// @todo Check that this state occurred after.
        self->_state = next;
        self->_timestamp.SetValue(next->GetTimestamp());
        self->_on_tick_callbacks.Call(next->GetTimestamp());
      }
    });
  }

  std::vector<rpc::Actor> Episode::GetActors() {
    auto state = GetState();
    auto actor_ids = state->GetActorIds();
    auto missing_ids = _actors.GetMissingIds(actor_ids);
    if (!missing_ids.empty()) {
      _actors.InsertRange(_client.GetActorsById(missing_ids));
    }
    return _actors.GetActorsById(actor_ids);
  }

} // namespace detail
} // namespace client
} // namespace carla
