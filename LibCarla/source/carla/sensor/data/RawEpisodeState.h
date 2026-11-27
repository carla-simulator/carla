// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/ListView.h"
#include "carla/sensor/data/ActorDynamicState.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/s11n/EpisodeStateSerializer.h"

#include <algorithm>
#include <optional>

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
      : Super(std::move(data), [](const RawData &raw) {
          const auto count = TrafficLightCount(raw);
          return (count && !IsDelta(raw)) ?
              Serializer::header_offset + *count * sizeof(TrafficLightDynamicState) : raw.size();
        }) {}

  private:

    auto GetHeader() const {
      return Serializer::DeserializeHeader(Super::GetRawData());
    }

    /// Number of traffic lights between the header and the actors, or nothing
    /// when the message cannot hold the count it gives (another build): then
    /// it reads as empty, never past its end.
    static std::optional<size_t> TrafficLightCount(const RawData &raw) {
      if (raw.size() < Serializer::header_offset) {
        return std::nullopt;
      }
      const size_t available = raw.size() - Serializer::header_offset;
      const size_t count = Serializer::DeserializeHeader(raw).traffic_light_count;
      const size_t size = count * sizeof(TrafficLightDynamicState);
      if ((size > available) || (!IsDelta(raw) && ((available - size) % sizeof(ActorDynamicState) != 0u))) {
        return std::nullopt;
      }
      return count;
    }

    static bool IsDelta(const RawData &raw) {
      return (raw.size() >= Serializer::header_offset) &&
          ((Serializer::DeserializeHeader(raw).simulation_state & Serializer::Delta) != 0u);
    }

  public:

    /// What changes of the traffic lights that are static actors, which are
    /// not among the actors of this state.
    auto GetTrafficLights() const {
      const auto &raw = Super::GetRawData();
      const auto *begin = reinterpret_cast<const TrafficLightDynamicState *>(
          raw.begin() + std::min(raw.size(), Serializer::header_offset));
      return MakeListView(begin, begin + TrafficLightCount(raw).value_or(0u));
    }

    /// True if this message holds only what changed since the previous frame; the actor array is then empty.
    bool IsDelta() const {
      return IsDelta(Super::GetRawData());
    }

    /// The actor entries of a delta (id, mask, ranges of the mask); nothing if not a delta or its lights do not fit.
    std::optional<ListView<const unsigned char *>> GetActorDeltas() const {
      const auto &raw = Super::GetRawData();
      const auto count = TrafficLightCount(raw);
      if (!count || !IsDelta(raw)) {
        return std::nullopt;
      }
      return MakeListView<const unsigned char *>(
          raw.begin() + Serializer::header_offset + *count * sizeof(TrafficLightDynamicState), raw.end());
    }

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

    geom::Vector3DInt GetMapOrigin() const {
      return GetHeader().map_origin;
    }

    /// Simulation state flags
    Serializer::SimulationState GetSimulationState() const {
      return GetHeader().simulation_state;
    }

  };

} // namespace data
} // namespace sensor
} // namespace carla
