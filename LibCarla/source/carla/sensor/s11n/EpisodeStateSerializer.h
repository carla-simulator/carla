// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/BufferView.h"
#include "carla/Debug.h"
#include "carla/Memory.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Vector3DInt.h"
#include "carla/sensor/RawData.h"
#include "carla/sensor/data/ActorDynamicState.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

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
      PendingLightUpdate = (0x1 << 1),
      /// Only what changed since the previous frame follows the header (see MakeDelta).
      Delta              = (0x1 << 2)
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

    /// Bounds of the 8 ranges of an ActorDynamicState that a delta sends apart, one bit of its mask each.
    constexpr static size_t delta_ranges[9] = {
        offsetof(data::ActorDynamicState, actor_state),
        offsetof(data::ActorDynamicState, transform),
        offsetof(data::ActorDynamicState, transform) + sizeof(geom::Location),
        offsetof(data::ActorDynamicState, velocity),
        offsetof(data::ActorDynamicState, angular_velocity),
        offsetof(data::ActorDynamicState, acceleration),
        offsetof(data::ActorDynamicState, state),
        offsetof(data::ActorDynamicState, state) + sizeof(data::detail::PackedVehicleControl),
        sizeof(data::ActorDynamicState)};

    /// Writes to @a delta what changed since @a previous; false if episode, ids or order differ or nothing is saved.
    static bool MakeDelta(const BufferView &previous, const BufferView &current, Buffer &delta) {
      using Light = data::TrafficLightDynamicState;
      using Actor = data::ActorDynamicState;
      const size_t size = current.size();
      const unsigned char *old = previous.data();
      const unsigned char *now = current.data();
      Header header;
      if ((previous.size() != size) || (size < sizeof(header))) {
        return false;
      }
      std::memcpy(&header, now, sizeof(header));
      const size_t actors = sizeof(header) + size_t(header.traffic_light_count) * sizeof(Light);
      if ((std::memcmp(old, now, sizeof(header.episode_id)) != 0) ||
          (reinterpret_cast<const Header *>(old)->traffic_light_count != header.traffic_light_count) ||
          (actors > size) || ((size - actors) % sizeof(Actor) != 0u)) {
        return false;
      }
      // The mask is the only thing a delta adds: one byte per actor at most.
      delta.reset(static_cast<Buffer::size_type>(size + (size - actors) / sizeof(Actor)));
      unsigned char *out = delta.data() + sizeof(header);
      header.traffic_light_count = 0u;
      size_t i = sizeof(header);
      // One loop per record size: only a memcmp of constant length is inlined, and a big map has a thousand lights.
      for (; i < actors; i += sizeof(Light)) {
        if (std::memcmp(old + i, now + i, sizeof(Light)) == 0) {
          continue;
        }
        if (std::memcmp(old + i, now + i, sizeof(ActorId)) != 0) {
          return false;
        }
        std::memcpy(out, now + i, sizeof(Light));
        out += sizeof(Light);
        ++header.traffic_light_count;
      }
      for (; i < size; i += sizeof(Actor)) {
        if (std::memcmp(old + i, now + i, sizeof(Actor)) == 0) {
          continue;
        }
        if (std::memcmp(old + i, now + i, sizeof(ActorId)) != 0) {
          return false;
        }
        std::memcpy(out, now + i, sizeof(ActorId));
        unsigned char &mask = out[sizeof(ActorId)];
        mask = 0u;
        out += sizeof(ActorId) + 1u;
        for (size_t range = 0u; range < 8u; ++range) {
          const size_t begin = delta_ranges[range];
          const size_t bytes = delta_ranges[range + 1u] - begin;
          if (std::memcmp(old + i + begin, now + i + begin, bytes) != 0) {
            mask = static_cast<unsigned char>(mask | (1u << range));
            std::memcpy(out, now + i + begin, bytes);
            out += bytes;
          }
        }
      }
      const size_t delta_size = static_cast<size_t>(out - delta.data());
      if (delta_size >= size) {
        return false;
      }
      header.simulation_state = static_cast<SimulationState>(header.simulation_state | Delta);
      std::memcpy(delta.data(), &header, sizeof(header));
      delta.resize(delta_size);
      return true;
    }

    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
