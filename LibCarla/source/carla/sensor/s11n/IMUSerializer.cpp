// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/IMUSerializer.h"
#include "carla/sensor/data/IMUMeasurement.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> IMUSerializer::Deserialize(RawData &&data) {
    return SharedPtr<SensorData>(new data::IMUMeasurement(std::move(data)));
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
