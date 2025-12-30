// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/SemanticLidarSerializer.h"
#include "carla/sensor/data/SemanticLidarMeasurement.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> SemanticLidarSerializer::Deserialize(RawData DESERIALIZE_DECL_DATA(data)) {
    return SharedPtr<data::SemanticLidarMeasurement>(
        new data::SemanticLidarMeasurement{DESERIALIZE_MOVE_DATA(data)});
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
