// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Iterator.h"
#include "carla/ListView.h"
#include "carla/NonCopyable.h"
#include "carla/client/Timestamp.h"
#include "carla/sensor/data/ActorDynamicState.h"
#include "carla/sensor/data/RawEpisodeState.h"

#include <memory>
#include <unordered_map>

namespace carla {
namespace client {
namespace detail {

  /// Represents the state of all the actors of an episode at a given frame.
  class EpisodeState
    : std::enable_shared_from_this<EpisodeState>,
      private NonCopyable {
  public:

    struct ActorState {
      geom::Transform transform;
      geom::Vector3D velocity;
      geom::Vector3D angular_velocity;
      geom::Vector3D acceleration;
      sensor::data::ActorDynamicState::TypeDependentState state;
    };

    explicit EpisodeState(uint64_t episode_id) : _episode_id(episode_id) {}

    explicit EpisodeState(const sensor::data::RawEpisodeState &state);

    auto GetEpisodeId() const {
      return _episode_id;
    }

    auto GetFrameCount() const {
      return _timestamp.frame_count;
    }

    const auto &GetTimestamp() const {
      return _timestamp;
    }

    ActorState GetActorState(ActorId id) const {
      ActorState state;
      auto it = _actors.find(id);
      if (it != _actors.end()) {
        state = it->second;
      } else {
        log_debug("actor", id, "not found in episode");
      }
      return state;
    }

    auto GetActorIds() const {
      return MakeListView(
          iterator::make_map_keys_const_iterator(_actors.begin()),
          iterator::make_map_keys_const_iterator(_actors.end()));
    }

  private:

    const uint64_t _episode_id;

    const Timestamp _timestamp;

    std::unordered_map<ActorId, ActorState> _actors;
  };

} // namespace detail
} // namespace client
} // namespace carla
