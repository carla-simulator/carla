// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/sensor/RawData.h"

namespace carla {
namespace sensor {

  class SensorData;

namespace s11n {

  // ===========================================================================
  // -- BenchmarkSerializer ---------------------------------------------------------
  // ===========================================================================

  class BenchmarkSerializer {
  public:
    template <typename Sensor>
    static Buffer Serialize(
        const Sensor &sensor,
        const std::string &result);

    static std::string DeserializeRawData(const RawData &data) {
      return MsgPack::UnPack<std::string>(data.begin(), data.size());
    }

    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

  template <typename Sensor>
  inline Buffer BenchmarkSerializer::Serialize(
      const Sensor &,
      const std::string &result) {
    return MsgPack::Pack(result);
  }


} // namespace s11n
} // namespace sensor
} // namespace carla
