// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Debug.h"
#include "carla/Memory.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Vector3DInt.h"
#include "carla/sensor/RawData.h"
#include "carla/sensor/data/ActorDynamicState.h"

#include <cstdint>

namespace carla {
namespace sensor {

  class SensorData;

namespace s11n {

  /// Serializes the current state of the whole episode.
  class EpisodeStateSerializer {
  public:

    enum SimulationState : uint32_t {
      None               = (0x0 << 0),
      MapChange          = (0x1 << 0),
      PendingLightUpdate = (0x1 << 1)
    };

    /// Header::simulation_state holds the SimulationState flags in its low
    /// byte and, in the other 24 bits, the version of the set of static
    /// actors. Static actors are not in the actor list that follows the header
    /// (see Episode::GetStaticActors); the server changes the version when
    /// their set changes and never sends 0.
    constexpr static uint32_t static_actors_version_shift = 8u;
    constexpr static uint32_t static_actors_version_max = 0xFFFFFFu;

#pragma pack(push, 1)
    struct Header {
      uint64_t episode_id;
      double platform_timestamp;
      float delta_seconds;
      geom::Vector3DInt map_origin;
      SimulationState simulation_state = SimulationState::None;
      /// The header is followed by this many TrafficLightDynamicState, one per
      /// traffic light that is a static actor, and then the ActorDynamicState
      /// of the other actors.
      uint32_t traffic_light_count = 0u;
    };
#pragma pack(pop)

    constexpr static auto header_offset = sizeof(Header);

    static const Header &DeserializeHeader(const RawData &message) {
      return *reinterpret_cast<const Header *>(message.begin());
    }

    template <typename SensorT>
    static Buffer Serialize(const SensorT &, Buffer &&buffer) {
      return std::move(buffer);
    }

    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
