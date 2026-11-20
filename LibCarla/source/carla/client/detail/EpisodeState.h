// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ListView.h"
#include "carla/NonCopyable.h"
#include "carla/client/ActorSnapshot.h"
#include "carla/client/Timestamp.h"
#include "carla/geom/Vector3DInt.h"
#include "carla/sensor/data/RawEpisodeState.h"

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
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
      return FindIndex(actor_id).has_value();
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

    /// Ids of the actors in this frame, in ascending order.
    auto GetActorIds() const {
      return MakeListView(_ids.begin(), _ids.end());
    }

    /// Position of @a id in GetActorIds(), if the actor is in this frame.
    std::optional<size_t> FindIndex(ActorId id) const {
      if (!_slots.empty()) {
        // Unsigned: an id below _first_id wraps past the end.
        const ActorId slot = id - _first_id;
        if ((slot < _slots.size()) && (_slots[slot] != 0u)) {
          return static_cast<size_t>(_slots[slot] - 1u);
        }
        return std::nullopt;
      }
      const auto it = std::lower_bound(_ids.begin(), _ids.end(), id);
      if ((it == _ids.end()) || (*it != id)) {
        return std::nullopt;
      }
      return static_cast<size_t>(it - _ids.begin());
    }

    /// Snapshot of the actor at @a index of GetActorIds().
    const ActorSnapshot &GetActorSnapshotAt(size_t index) const {
      return _actors[index];
    }

    /// Location in meters, in double, of the actor at @a index of
    /// GetActorIds(). ActorSnapshot::transform holds it rounded to float.
    const std::array<double, 3u> &GetLocationAt(size_t index) const {
      return _locations[index];
    }

    size_t size() const {
      return _actors.size();
    }

    /// Iterates the actor snapshots in ascending id order.
    auto begin() const {
      return _actors.cbegin();
    }

    auto end() const {
      return _actors.cend();
    }

  private:

    template <typename T>
    void CopyActorSnapshotIfPresent(ActorId id, T &value) const {
      const auto index = FindIndex(id);
      if (index.has_value()) {
        value = _actors[*index];
      }
    }

    const uint64_t _episode_id;

    const Timestamp _timestamp;

    geom::Vector3DInt _map_origin;

    SimulationState _simulation_state;

    // Three arrays in the same ascending id order.
    std::vector<ActorId> _ids;

    std::vector<ActorSnapshot> _actors;

    std::vector<std::array<double, 3u>> _locations;

    // Actor ids are handed out in increasing order, so they are usually dense:
    // then _slots[id - _first_id] is the actor's index + 1 (0 if absent), and
    // a lookup is one array access. Empty when the ids are too sparse; then a
    // lookup is a binary search in _ids.
    ActorId _first_id = 0u;

    std::vector<uint32_t> _slots;
  };

} // namespace detail
} // namespace client
} // namespace carla
