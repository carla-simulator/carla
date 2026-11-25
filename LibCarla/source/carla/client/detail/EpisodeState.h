// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Iterator.h"
#include "carla/ListView.h"
#include "carla/NonCopyable.h"
#include "carla/client/ActorSnapshot.h"
#include "carla/client/Timestamp.h"
#include "carla/geom/Vector3DInt.h"
#include "carla/sensor/data/RawEpisodeState.h"

#include <optional>

#include <memory>
#include <unordered_map>

namespace carla {
namespace client {
namespace detail {

  /// Represents the state of all the actors of an episode at a given frame.
  class EpisodeState
    : public std::enable_shared_from_this<EpisodeState>,
      private NonCopyable {

      using SimulationState = sensor::s11n::EpisodeStateSerializer::SimulationState;

  public:

    explicit EpisodeState(uint64_t episode_id) : _episode_id(episode_id) {}

    explicit EpisodeState(const sensor::data::RawEpisodeState &state);

    auto GetEpisodeId() const {
      return _episode_id;
    }

    auto GetFrame() const {
      return _timestamp.frame;
    }

    const auto &GetTimestamp() const {
      return _timestamp;
    }

    SimulationState GetsimulationState() const {
      return _simulation_state;
    }

    bool HasMapChanged() const {
      return (_simulation_state & SimulationState::MapChange) != SimulationState::None;
    }

    bool IsLightUpdatePending() const {
      return (_simulation_state & SimulationState::PendingLightUpdate)  != 0;
    }

    /// Version of the set of static actors, which are not in this state (see
    /// Episode::GetStaticActors). 0 until the first state arrives.
    uint32_t GetStaticActorsVersion() const {
      return static_cast<uint32_t>(_simulation_state) >>
          sensor::s11n::EpisodeStateSerializer::static_actors_version_shift;
    }

    bool ContainsActorSnapshot(ActorId actor_id) const {
      return _actors.find(actor_id) != _actors.end();
    }

    ActorSnapshot GetActorSnapshot(ActorId id) const {
      ActorSnapshot state;
      CopyActorSnapshotIfPresent(id, state);
      return state;
    }

    std::optional<ActorSnapshot> GetActorSnapshotIfPresent(ActorId id) const {
      std::optional<ActorSnapshot> state;
      CopyActorSnapshotIfPresent(id, state);
      return state;
    }

    /// State, elapsed time and frozen flag of a traffic light that is a
    /// static actor; nothing for any other actor.
    std::optional<sensor::data::TrafficLightDynamicState> GetTrafficLightState(ActorId id) const {
      auto it = _traffic_lights.find(id);
      if (it == _traffic_lights.end()) {
        return std::nullopt;
      }
      return it->second;
    }

    auto GetActorIds() const {
      return MakeListView(
          iterator::make_map_keys_const_iterator(_actors.begin()),
          iterator::make_map_keys_const_iterator(_actors.end()));
    }

    size_t size() const {
      return _actors.size();
    }

    auto begin() const {
      return iterator::make_map_values_const_iterator(_actors.begin());
    }

    auto end() const {
      return iterator::make_map_values_const_iterator(_actors.end());
    }

  private:

    template <typename T>
    void CopyActorSnapshotIfPresent(ActorId id, T &value) const {
      auto it = _actors.find(id);
      if (it != _actors.end()) {
        value = it->second;
      }
    }

    const uint64_t _episode_id;

    const Timestamp _timestamp;

    geom::Vector3DInt _map_origin;

    SimulationState _simulation_state = SimulationState::None;

    std::unordered_map<ActorId, ActorSnapshot> _actors;

    std::unordered_map<ActorId, sensor::data::TrafficLightDynamicState> _traffic_lights;
  };

} // namespace detail
} // namespace client
} // namespace carla
