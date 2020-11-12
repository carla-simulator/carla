// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/sensor/RawData.h"
#include "carla/sensor/data/BenchmarkData.h"

namespace carla {
namespace sensor {

  class BenchmarkData;

namespace s11n {

  // ===========================================================================
  // -- BenchmarkData ---------------------------------------------------------
  // ===========================================================================

  class BenchmarkSerializer {
  public:
    template <typename Sensor>
    static Buffer Serialize(
        const Sensor &sensor,
        const data::BenchmarkData &measurement,
        Buffer &&output);

    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

  template <typename Sensor>
  inline Buffer BenchmarkSerializer::Serialize(
      const Sensor &,
      const data::BenchmarkData &measurement,
      Buffer &&output) {
    output.copy_from(measurement._benchmark_result);
    return std::move(output);
  }


} // namespace s11n
} // namespace sensor
} // namespace carla