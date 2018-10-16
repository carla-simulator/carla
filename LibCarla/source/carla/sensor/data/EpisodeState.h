// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/sensor/data/ActorState.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/s11n/EpisodeStateSerializer.h"

namespace carla {
namespace sensor {
namespace data {

  /// State of the episode at a given frame.
  class EpisodeState : public Array<ActorState>  {
    using Super = Array<ActorState>;
  protected:

    using Serializer = s11n::EpisodeStateSerializer;

    friend Serializer;

    explicit EpisodeState(RawData data)
      : Super(std::move(data)) {
      Super::SetOffset(Serializer::header_offset);
    }

  private:

    auto GetHeader() const {
      return Serializer::DeserializeHeader(Super::GetRawData());
    }

  public:

    /// Simulation time-stamp, simulated seconds elapsed since the beginning of
    /// the current episode.
    double GetGameTimeStamp() const {
      return GetHeader().game_timestamp;
    }

    /// Time-stamp of the frame at which this measurement was taken, in seconds
    /// as given by the OS.
    double GetPlatformTimeStamp() const {
      return GetHeader().platform_timestamp;
    }
  };

} // namespace data
} // namespace sensor
} // namespace carla
