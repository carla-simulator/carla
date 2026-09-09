// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/DistanceImageSerializer.h"

#include "carla/sensor/data/Image.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> DistanceImageSerializer::Deserialize(RawData &&data) {
    auto image = SharedPtr<data::DistanceImage>(new data::DistanceImage{std::move(data)});
    return image;
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
