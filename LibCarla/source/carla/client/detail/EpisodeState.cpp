// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/EpisodeState.h"

#include <algorithm>
#include <cstring>
#include <utility>

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
    const size_t count = state.size();
    // The server lists the actors in its registry's order: put them in id
    // order, through the slot table when the ids are dense enough, else by
    // sorting.
    std::vector<size_t> order;
    order.reserve(count);
    if (count > 0u) {
      ActorId lowest = state[0u].id;
      ActorId highest = lowest;
      for (size_t i = 1u; i < count; ++i) {
        lowest = std::min(lowest, state[i].id);
        highest = std::max(highest, state[i].id);
      }
      const size_t range = static_cast<size_t>(highest - lowest) + 1u;
      if (range <= 4u * count + 1024u) {
        _first_id = lowest;
        _slots.assign(range, 0u);
        for (size_t i = 0u; i < count; ++i) {
          auto &slot = _slots[state[i].id - lowest];
          DEBUG_ASSERT(slot == 0u);
          slot = static_cast<uint32_t>(i + 1u);
        }
        for (auto &slot : _slots) {
          if (slot != 0u) {
            order.push_back(slot - 1u);
            // From now on the slot holds the index in id order.
            slot = static_cast<uint32_t>(order.size());
          }
        }
      } else {
        for (size_t i = 0u; i < count; ++i) {
          order.push_back(i);
        }
        std::sort(order.begin(), order.end(), [&state](size_t lhs, size_t rhs) {
          return state[lhs].id < state[rhs].id;
        });
      }
    }
    _ids.reserve(count);
    _actors.reserve(count);
    _locations.reserve(count);
    for (const size_t entry : order) {
      const auto &actor = state[entry];
      DEBUG_ASSERT(_ids.empty() || (_ids.back() < actor.id));
      // The server sends Unreal's location, centimeters in double. The float
      // in meters is rounded exactly as geom::Location(FVector) rounds it.
      double cm[3u];
      std::memcpy(cm, actor.location, sizeof(cm));
      const geom::Location location{
          static_cast<float>(1e-2f * cm[0u]),
          static_cast<float>(1e-2f * cm[1u]),
          static_cast<float>(1e-2f * cm[2u])};
      _ids.push_back(actor.id);
      _actors.push_back(ActorSnapshot{
          actor.id,
          actor.actor_state,
          geom::Transform{location, actor.rotation},
          actor.velocity,
          actor.angular_velocity,
          actor.acceleration,
          actor.state});
      _locations.push_back({cm[0u] / 1e2, cm[1u] / 1e2, cm[2u] / 1e2});
    }
  }

} // namespace detail
} // namespace client
} // namespace carla
