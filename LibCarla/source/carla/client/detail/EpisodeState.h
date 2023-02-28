// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
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
#include "carla/sensor/data/RawEpisodeState.h"

#include <boost/optional.hpp>

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

    bool ContainsActorSnapshot(ActorId actor_id) const {
      return _actors.find(actor_id) != _actors.end();
    }

    ActorSnapshot GetActorSnapshot(ActorId id) const {
      ActorSnapshot state;
      CopyActorSnapshotIfPresent(id, state);
      return state;
    }

    boost::optional<ActorSnapshot> GetActorSnapshotIfPresent(ActorId id) const {
      boost::optional<ActorSnapshot> state;
      CopyActorSnapshotIfPresent(id, state);
      return state;
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

    SimulationState _simulation_state;

    std::unordered_map<ActorId, ActorSnapshot> _actors;
  };

} // namespace detail
} // namespace client
} // namespace carla
