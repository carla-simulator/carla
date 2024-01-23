// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/data/CollisionEvent.h"
#include "carla/sensor/s11n/CollisionEventSerializer.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> CollisionEventSerializer::Deserialize(RawData &&data) {
    return SharedPtr<SensorData>(new data::CollisionEvent(std::move(data)));
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
