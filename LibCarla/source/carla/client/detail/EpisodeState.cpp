// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/EpisodeState.h"

#include <cstddef>
#include <cstring>

namespace carla {
namespace client {
namespace detail {

  EpisodeState::EpisodeState(const sensor::data::RawEpisodeState &state)
    : _episode_id(state.GetEpisodeId()),
      _timestamp(
          state.GetFrame(),
          state.GetGameTimeStamp(),
          state.GetDeltaSeconds(),
          state.GetPlatformTimeStamp()),
      _map_origin(state.GetMapOrigin()),
      _simulation_state(state.GetSimulationState()) {
    _actors.reserve(state.size());
    for (auto &&actor : state) {
      _actors.push_back(
          ActorSnapshot{
              actor.id,
              actor.actor_state,
              actor.transform,
              actor.velocity,
              actor.angular_velocity,
              actor.acceleration,
              actor.state});
    }
    const auto traffic_lights = state.GetTrafficLights();
    _traffic_lights.assign(traffic_lights.begin(), traffic_lights.end());
    const auto index = [](const auto &items) {
      auto fresh = std::make_shared<Index>();
      fresh->positions.reserve(items.size());
      for (size_t i = 0u; i < items.size(); ++i) {
        const ActorId id = items[i].id;
        fresh->positions.emplace(id, static_cast<uint32_t>(i));
      }
      return fresh;
    };
    _actor_index = index(_actors);
    _traffic_light_index = index(_traffic_lights);
  }

  EpisodeState::EpisodeState(const sensor::data::RawEpisodeState &delta, const EpisodeState &previous)
    : _episode_id(delta.GetEpisodeId()),
      _timestamp(
          delta.GetFrame(),
          delta.GetGameTimeStamp(),
          delta.GetDeltaSeconds(),
          delta.GetPlatformTimeStamp()),
      _map_origin(delta.GetMapOrigin()),
      _simulation_state(delta.GetSimulationState()),
      _actors(previous._actors),
      _traffic_lights(previous._traffic_lights),
      _actor_index(previous._actor_index),
      _traffic_light_index(previous._traffic_light_index) {}

  std::shared_ptr<const EpisodeState> EpisodeState::Make(
      const sensor::data::RawEpisodeState &state,
      const std::shared_ptr<const EpisodeState> &previous) {
    if (!state.IsDelta()) {
      return std::make_shared<const EpisodeState>(state);
    }
    if ((previous == nullptr) || (previous->GetEpisodeId() != state.GetEpisodeId()) ||
        (previous->GetFrame() + 1u != state.GetFrame())) {
      return nullptr;
    }
    std::shared_ptr<EpisodeState> next(new EpisodeState(state, *previous));
    if (!next->Patch(state)) {
      return nullptr;
    }
    return next;
  }

  bool EpisodeState::Patch(const sensor::data::RawEpisodeState &delta) {
    using Serializer = sensor::s11n::EpisodeStateSerializer;
    const auto entries = delta.GetActorDeltas();
    if (!entries.has_value()) {
      return false;
    }
    // Where each range of Serializer::delta_ranges is in an ActorSnapshot.
    constexpr size_t snapshot_ranges[8] = {
        offsetof(ActorSnapshot, actor_state),
        offsetof(ActorSnapshot, transform),
        offsetof(ActorSnapshot, transform) + sizeof(geom::Location),
        offsetof(ActorSnapshot, velocity),
        offsetof(ActorSnapshot, angular_velocity),
        offsetof(ActorSnapshot, acceleration),
        offsetof(ActorSnapshot, state),
        offsetof(ActorSnapshot, state) + Serializer::delta_ranges[7] - Serializer::delta_ranges[6]};
    // A delta lists lights and actors in the order of the whole message, which is the order of the vectors.
    size_t position = 0u;
    for (auto &&light : delta.GetTrafficLights()) {
      const ActorId id = light.id;
      while ((position < _traffic_lights.size()) && (_traffic_lights[position].id != id)) {
        ++position;
      }
      if (position == _traffic_lights.size()) {
        return false;
      }
      _traffic_lights[position] = light;
    }
    position = 0u;
    const unsigned char *in = entries->begin();
    const unsigned char *const end = entries->end();
    while (in != end) {
      ActorId id;
      if (static_cast<size_t>(end - in) < sizeof(id) + 1u) {
        return false;
      }
      std::memcpy(&id, in, sizeof(id));
      const unsigned char mask = in[sizeof(id)];
      in += sizeof(id) + 1u;
      while ((position < _actors.size()) && (_actors[position].id != id)) {
        ++position;
      }
      if (position == _actors.size()) {
        return false;
      }
      auto *actor = reinterpret_cast<unsigned char *>(&_actors[position]);
      for (size_t range = 0u; range < 8u; ++range) {
        if ((mask & (1u << range)) != 0u) {
          const size_t length = Serializer::delta_ranges[range + 1u] - Serializer::delta_ranges[range];
          if (static_cast<size_t>(end - in) < length) {
            return false;
          }
          std::memcpy(actor + snapshot_ranges[range], in, length);
          in += length;
        }
      }
    }
    return true;
  }

} // namespace detail
} // namespace client
} // namespace carla
