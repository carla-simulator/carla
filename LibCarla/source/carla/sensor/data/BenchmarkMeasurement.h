// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/s11n/BenchmarkSerializer.h"

namespace carla {
namespace sensor {
namespace data {

  class BenchmarkMeasurement : public SensorData {
    using Super = SensorData;
  protected:

    using Serializer = s11n::BenchmarkSerializer;

    friend Serializer;

    explicit BenchmarkMeasurement(RawData &&data)
      : Super(data),
        _result(Serializer::DeserializeRawData(data)) {}

  public:
    std::string GetResult() const
    {
      return _result;
    }

  private:
    std::string _result;

  };

} // namespace data
} // namespace sensor
} // namespace carla
