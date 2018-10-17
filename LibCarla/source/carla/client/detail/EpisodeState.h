// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
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
      geom::Vector3D acceleration;
    };

    /// @copydoc carla::sensor::SensorData::GetFrameNumber()
    size_t GetFrameNumber() const {
      return _frame_number;
    }

    /// @copydoc carla::sensor::data::RawEpisodeState::GetGameTimeStamp()
    double GetGameTimeStamp() const {
      return _game_timestamp;
    }

    /// @copydoc carla::sensor::data::RawEpisodeState::GetPlatformTimeStamp()
    double GetPlatformTimeStamp() const {
      return _platform_timestamp;
    }

    ActorState GetActorState(actor_id_type id) const {
      ActorState state;
      auto it = _actors.find(id);
      if (it != _actors.end()) {
        state = it->second;
      } else {
        log_debug("actor", id, "not found in episode");
      }
      return state;
    }

    std::shared_ptr<const EpisodeState> DeriveNextStep(
        const sensor::data::RawEpisodeState &state) const;

  private:

    size_t _frame_number = 0.0;

    double _game_timestamp = 0.0;

    double _platform_timestamp = 0.0;

    std::unordered_map<actor_id_type, ActorState> _actors;
  };

} // namespace detail
} // namespace client
} // namespace carla
