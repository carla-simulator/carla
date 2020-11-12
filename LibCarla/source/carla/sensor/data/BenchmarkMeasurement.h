// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/s11n/BenchmarkSerializer.h"
#include "carla/sensor/data/BenchmarkData.h"

namespace carla {
namespace sensor {
namespace data {

  class BenchmarkMeasurement : public Array<data::BenchmarkResult> {
    using Super = Array<data::BenchmarkResult>;
  protected:

    using Serializer = s11n::BenchmarkSerializer;

    friend Serializer;

    explicit BenchmarkMeasurement(RawData &&data)
      : Super(0u, std::move(data)) {}

  public:

    Super::size_type GetDetectionAmount() const {
      return Super::size();
    }
  };

} // namespace data
} // namespace sensor
} // namespace carla
