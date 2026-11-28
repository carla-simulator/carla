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
#include <memory_resource>
#include <unordered_map>
#include <vector>

namespace carla {
namespace client {
namespace detail {

  /// Represents the state of all the actors of an episode at a given frame.
  class EpisodeState
    : public std::enable_shared_from_this<EpisodeState>,
      private NonCopyable {

      using SimulationState = sensor::s11n::EpisodeStateSerializer::SimulationState;

  public:

    explicit EpisodeState(uint64_t episode_id)
      : _episode_id(episode_id),
        _actor_index(std::make_shared<Index>()),
        _traffic_light_index(_actor_index) {}

    explicit EpisodeState(const sensor::data::RawEpisodeState &state);

    /// The state of @a state; a delta is applied on @a previous, and gives null if it does not follow it.
    static std::shared_ptr<const EpisodeState> Make(
        const sensor::data::RawEpisodeState &state,
        const std::shared_ptr<const EpisodeState> &previous);

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
      return _actor_index->positions.find(actor_id) != _actor_index->positions.end();
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
      auto it = _traffic_light_index->positions.find(id);
      if (it == _traffic_light_index->positions.end()) {
        return std::nullopt;
      }
      return _traffic_lights[it->second];
    }

    auto GetActorIds() const {
      return MakeListView(
          iterator::make_map_keys_const_iterator(_actor_index->positions.begin()),
          iterator::make_map_keys_const_iterator(_actor_index->positions.end()));
    }

    size_t size() const {
      return _actors.size();
    }

    auto begin() const {
      return _actors.begin();
    }

    auto end() const {
      return _actors.end();
    }

  private:

    /// A copy of @a previous with the header of @a delta; Patch then applies what changed.
    EpisodeState(const sensor::data::RawEpisodeState &delta, const EpisodeState &previous);

    bool Patch(const sensor::data::RawEpisodeState &delta);

    template <typename T>
    void CopyActorSnapshotIfPresent(ActorId id, T &value) const {
      auto it = _actor_index->positions.find(id);
      if (it != _actor_index->positions.end()) {
        value = _actors[it->second];
      }
    }

    /// Never changed once built; the nodes of the map come from one buffer instead of one allocation each.
    struct Index {
      std::pmr::monotonic_buffer_resource memory;
      std::pmr::unordered_map<ActorId, uint32_t> positions{&memory};
    };

    const uint64_t _episode_id;

    const Timestamp _timestamp;

    geom::Vector3DInt _map_origin;

    SimulationState _simulation_state = SimulationState::None;

    /// In the order of the last whole message.
    std::vector<ActorSnapshot> _actors;

    std::vector<sensor::data::TrafficLightDynamicState> _traffic_lights;

    /// Position in the vectors by id; a state made from a delta shares them with the state before it.
    std::shared_ptr<const Index> _actor_index;

    std::shared_ptr<const Index> _traffic_light_index;
  };

} // namespace detail
} // namespace client
} // namespace carla
