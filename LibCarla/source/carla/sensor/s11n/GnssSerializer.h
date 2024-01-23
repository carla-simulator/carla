// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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

    static geom::GeoLocation DeserializeRawData(const RawData &message) {
      return MsgPack::UnPack<geom::GeoLocation>(message.begin(), message.size());
    }

    template <typename SensorT>
    static Buffer Serialize(
        const SensorT &,
        const geom::GeoLocation &geo_location
        ) {
      return MsgPack::Pack(geo_location);
    }

    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
