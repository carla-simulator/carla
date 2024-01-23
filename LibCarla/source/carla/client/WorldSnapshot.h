// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Timestamp.h"
#include "carla/client/ActorSnapshot.h"
#include "carla/client/detail/EpisodeState.h"

#include <optional>

namespace carla {
namespace client {

  class WorldSnapshot {
  public:

    WorldSnapshot(std::shared_ptr<const detail::EpisodeState> state)
      : _state(std::move(state)) {}

    /// Get the id of the episode associated with this world.
    uint64_t GetId() const {
      return _state->GetEpisodeId();
    }

    size_t GetFrame() const {
      return GetTimestamp().frame;
    }

    /// Get timestamp of this snapshot.
    const Timestamp &GetTimestamp() const {
      return _state->GetTimestamp();
    }

    /// Check if an actor is present in this snapshot.
    bool Contains(ActorId actor_id) const {
      return _state->ContainsActorSnapshot(actor_id);
    }

    /// Find an ActorSnapshot by id.
    std::optional<ActorSnapshot> Find(ActorId actor_id) const {
      return _state->GetActorSnapshotIfPresent(actor_id);
    }

    /// Return number of ActorSnapshots present in this WorldSnapshot.
    size_t size() const {
      return _state->size();
    }

    /// Return a begin iterator to the list of ActorSnapshots.
    auto begin() const {
      return _state->begin();
    }

    /// Return a past-the-end iterator to the list of ActorSnapshots.
    auto end() const {
      return _state->end();
    }

    bool operator==(const WorldSnapshot &rhs) const {
      return GetTimestamp() == rhs.GetTimestamp();
    }

    bool operator!=(const WorldSnapshot &rhs) const {
      return !(*this == rhs);
    }

  private:

    std::shared_ptr<const detail::EpisodeState> _state;
  };

} // namespace client
} // namespace carla
