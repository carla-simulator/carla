// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/sensor/data/ActorDynamicState.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/s11n/EpisodeStateSerializer.h"

namespace carla {
namespace sensor {
namespace data {

  /// State of the episode at a given frame.
  class RawEpisodeState : public Array<ActorDynamicState> {
    using Super = Array<ActorDynamicState>;
  protected:

    using Serializer = s11n::EpisodeStateSerializer;

    friend Serializer;

    explicit RawEpisodeState(RawData &&data)
      : Super(Serializer::header_offset, std::move(data)) {}

  private:

    auto GetHeader() const {
      return Serializer::DeserializeHeader(Super::GetRawData());
    }

  public:

    /// Unique id of the episode at which this data was generated.
    uint64_t GetEpisodeId() const {
      return GetHeader().episode_id;
    }

    /// Simulation time-stamp, simulated seconds elapsed since the beginning of
    /// the current episode.
    double GetGameTimeStamp() const {
      return GetTimestamp();
    }

    /// Time-stamp of the frame at which this measurement was taken, in seconds
    /// as given by the OS.
    double GetPlatformTimeStamp() const {
      return GetHeader().platform_timestamp;
    }

    /// Simulated seconds elapsed since previous frame.
    double GetDeltaSeconds() const {
      return GetHeader().delta_seconds;
    }

    /// Simulation state flags
    Serializer::SimulationState GetSimulationState() const {
      return GetHeader().simulation_state;
    }

  };

} // namespace data
} // namespace sensor
} // namespace carla
