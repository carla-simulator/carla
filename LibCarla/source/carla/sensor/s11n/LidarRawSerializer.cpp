// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/LidarRawSerializer.h"
#include "carla/sensor/data/LidarRawMeasurement.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> LidarRawSerializer::Deserialize(RawData &&data) {
    return SharedPtr<data::LidarRawMeasurement>(
        new data::LidarRawMeasurement{std::move(data)});
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
