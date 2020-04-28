// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Debug.h"
#include "carla/Memory.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"
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

    enum SimulationState {
      None               = (0x0 << 0),
      MapChange          = (0x1 << 0),
      PendingLightUpdate = (0x1 << 1)
    };

#pragma pack(push, 1)
    struct Header {
      uint64_t episode_id;
      double platform_timestamp;
      float delta_seconds;
      SimulationState simulation_state = SimulationState::None;
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
