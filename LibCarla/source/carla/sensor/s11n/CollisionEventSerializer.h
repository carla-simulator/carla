// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Debug.h"
#include "carla/Memory.h"
#include "carla/rpc/Actor.h"
#include "carla/geom/Vector3D.h"
#include "carla/sensor/RawData.h"

namespace carla {
namespace sensor {

  class SensorData;

namespace s11n {

  /// Serializes the current state of the whole episode.
  class CollisionEventSerializer {
  public:

    struct Data {

      rpc::Actor self_actor;

      rpc::Actor other_actor;

      geom::Vector3D normal_impulse;

      MSGPACK_DEFINE_ARRAY(self_actor, other_actor, normal_impulse)
    };

    constexpr static auto header_offset = 0u;

    static Data DeserializeRawData(const RawData &message) {
      return MsgPack::UnPack<Data>(message.begin(), message.size());
    }

    template <typename SensorT>
    static Buffer Serialize(
        const SensorT &,
        rpc::Actor self_actor,
        rpc::Actor other_actor,
        geom::Vector3D normal_impulse) {
      return MsgPack::Pack(Data{self_actor, other_actor, normal_impulse});
    }

    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
