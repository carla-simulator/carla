#pragma once

#include "carla/Memory.h"
#include "carla/rpc/ActorId.h"
#include "carla/sensor/RawData.h"

#include <cstdint>
#include <cstring>

namespace carla {
namespace sensor {

  class SensorData;

namespace s11n {
  class GimbalSerializer {
  public:

    template <typename SensorT, typename EpisodeT, typename ActorListT>
    static Buffer Serialize(
        const SensorT &,
        const EpisodeT &episode,
        const ActorListT &detected_actors) {
      const uint32_t size_in_bytes = sizeof(ActorId) * detected_actors.Num();
      Buffer buffer{size_in_bytes};
      unsigned char *it = buffer.data();
      for (auto *actor : detected_actors) {
        ActorId id = episode.FindCarlaActor(actor)->GetActorId();
        std::memcpy(it, &id, sizeof(ActorId));
        it += sizeof(ActorId);
      }
      return buffer;
    }
    
    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

} // namespace s11n
} // namespace sensor
} // namespace carla