// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Debug.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"

class FActorRegistry;

namespace carla {
namespace sensor {
namespace s11n {

  /// Serializes the current state of the whole episode.
  class EpisodeStateSerializer {
  public:

#pragma pack(push, 1)

    struct Header {
      uint64_t frame_number;
      double game_timestamp;
      double platform_timestamp;
      uint64_t number_of_actors;
    };

    struct ActorInfo {
      uint32_t id;
      geom::Transform transform;
      geom::Vector3D velocity;
    };

#pragma pack(pop)

    constexpr static auto header_offset = sizeof(Header);

    static const Header &DeserializeHeader(const Buffer &message) {
      return *reinterpret_cast<const Header *>(message.data());
    }

    template <typename SensorT, typename ActorRegistryT>
    static Buffer Serialize(
        const SensorT &sensor,
        Buffer buffer,
        uint64_t frame_number,
        double game_timestamp,
        double platform_timestamp,
        const ActorRegistryT &actor_registry);

    static SharedPtr<SensorData> Deserialize(RawData data);
  };

#ifdef LIBCARLA_INCLUDED_FROM_UE4

  template <typename SensorT, typename ActorRegistryT>
  Buffer EpisodeStateSerializer::Serialize(
      const SensorT &,
      Buffer buffer,
      uint64_t frame_number,
      double game_timestamp,
      double platform_timestamp,
      const ActorRegistryT &actor_registry) {
    uint64_t number_of_actors =  actor_registry.Num();
    // Set up buffer for writing.
    buffer.reset(sizeof(Header) + sizeof(ActorInfo) * number_of_actors);
    auto begin = buffer.begin();
    auto write_data = [&begin](const auto &data) {
      std::memcpy(begin, &data, sizeof(data));
      begin += sizeof(data);
    };
    // Write header.
    Header header = {
      frame_number,
      game_timestamp,
      platform_timestamp,
      number_of_actors
    };
    write_data(header);
    // Write every actor.
    for (auto &&pair : actor_registry) {
      auto &&actor_view = pair.second;
      DEBUG_ASSERT(actor_view.GetActor() != nullptr);
      constexpr float TO_METERS = 1e-3;
      const auto velocity = TO_METERS * actor_view.GetActor()->GetVelocity();
      ActorInfo info = {
        actor_view.GetActorId(),
        actor_view.GetActor()->GetActorTransform(),
        geom::Vector3D{velocity.X, velocity.Y, velocity.Z}
      };
      write_data(info);
    }
    DEBUG_ASSERT(begin == buffer.end());
    return buffer;
  }

#endif // LIBCARLA_INCLUDED_FROM_UE4

} // namespace s11n
} // namespace sensor
} // namespace carla
