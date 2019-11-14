// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/geom/GeoLocation.h"
#include "carla/rpc/ActorId.h"
#include "carla/sensor/RawData.h"

#include <cstdint>
#include <cstring>

namespace carla {
namespace sensor {

  class SensorData;

namespace s11n {

  class GnssSerializer {
  public:

    struct Data {

      geom::GeoLocation geo_location;

      MSGPACK_DEFINE_ARRAY(geo_location);
    };

    static Data DeserializeRawData(const RawData &message) {
      return MsgPack::UnPack<Data>(message.begin(), message.size());
    }

    template <typename SensorT>
    static Buffer Serialize(
        const SensorT &,
        const geom::GeoLocation &geo_location
        ) {
      return MsgPack::Pack(Data{geo_location});
    }

    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
