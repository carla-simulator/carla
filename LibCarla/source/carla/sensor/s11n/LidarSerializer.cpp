// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#include "carla/sensor/data/LidarMeasurement.h"
#include "carla/sensor/s11n/LidarSerializer.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> LidarSerializer::Deserialize(RawData &&data) {
    return SharedPtr<data::LidarMeasurement>(
        new data::LidarMeasurement{std::move(data)});
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
