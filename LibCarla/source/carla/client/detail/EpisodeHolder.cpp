// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/EpisodeHolder.h"

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

  EpisodeHolder::EpisodeHolder(Client &client)
    : EpisodeHolder(client, client.GetEpisodeInfo()) {}

  EpisodeHolder::EpisodeHolder(Client &client, const rpc::EpisodeInfo &info)
    : _client(client),
      _episode(std::make_shared<Episode>(info.id)),
      _state(std::make_shared<EpisodeState>(info.id)),
      _token(info.token) {}

  EpisodeHolder::~EpisodeHolder() {
    try {
      _client.UnSubscribeFromStream(_token);
    } catch (const std::exception &e) {
      log_error("exception trying to disconnect from episode:", e.what());
    }
  }

  void EpisodeHolder::Listen() {
    std::weak_ptr<EpisodeHolder> weak = shared_from_this();
    _client.SubscribeToStream(_token, [weak](auto buffer) {
      auto self = weak.lock();
      if (self != nullptr) {
        auto data = sensor::Deserializer::Deserialize(std::move(buffer));
        const auto &raw_data = CastData(*data);
        const auto episode_id = raw_data.GetEpisodeId();

        std::shared_ptr<const EpisodeState> prev;

        if (episode_id != self->GetEpisode()->GetId()) {
          /// @todo this operation is not atomic, we could potentially end up
          /// resetting the episode several times.
          self->_episode.reset(std::make_shared<Episode>(episode_id));
          prev = std::make_shared<EpisodeState>(episode_id);
        } else {
          prev = self->_state.load();
        }

        auto next = prev->DeriveNextStep(raw_data);
        self->_state = next;
        self->_timestamp.SetValue(next->GetTimestamp());
        self->_on_tick_callbacks.Call(next->GetTimestamp());
      }
    });
  }

  std::vector<rpc::Actor> EpisodeHolder::GetActors() {
    auto episode = GetEpisode();
    auto state = GetState();
    if (episode->GetId() != state->GetEpisodeId()) {
      // This should never happen...
      log_error("Error retrieving actors: invalid episode.");
      return {};
    }
    return episode->GetActors(_client, state->GetActorIds());
  }

} // namespace detail
} // namespace client
} // namespace carla
