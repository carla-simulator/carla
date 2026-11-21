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

    /// First bytes of every snapshot body, "CEPS" in memory order. A client
    /// and a server of different wire formats find out on the first message.
    constexpr static uint32_t kMagic = 0x53504543u;

    /// Version of the snapshot wire format.
    constexpr static uint16_t kVersion = 2u;

    /// What follows the header.
    enum Kind : uint8_t {
      /// Every actor record, in any order.
      Keyframe = 0u,
      /// The changes against the body of sequence base_sequence, see
      /// episode_codec.
      Delta    = 1u
    };

#pragma pack(push, 1)
    struct Header {
      uint32_t magic = kMagic;
      uint16_t version = kVersion;
      uint8_t kind = Kind::Keyframe;
      uint8_t flags = 0u;
      uint64_t episode_id = 0u;
      double platform_timestamp = 0.0;
      float delta_seconds = 0.0f;
      geom::Vector3DInt map_origin;
      SimulationState simulation_state = SimulationState::None;
      /// Actors in the whole frame (also in a delta).
      uint32_t actor_count = 0u;
      /// Bytes after this header.
      uint32_t payload_size = 0u;
      /// Number of this body, from 1, never repeated by a server.
      uint64_t sequence = 0u;
      /// The body a delta applies to; 0 in a keyframe.
      uint64_t base_sequence = 0u;
      /// Sum, modulo 2^64, of episode_codec::RecordHash of every actor record
      /// of the whole frame.
      uint64_t state_hash = 0u;
    };
#pragma pack(pop)

    static_assert(sizeof(Header) == 76u, "the snapshot header is part of the wire format");

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
