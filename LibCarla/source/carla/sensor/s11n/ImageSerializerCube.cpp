// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/ImageSerializerCube.h"

#include "carla/sensor/data/ImageCube.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> ImageSerializerCube::Deserialize(RawData &&data) {
    auto image = SharedPtr<data::ImageCube>(new data::ImageCube{std::move(data)});
    // Set alpha of each pixel in the buffer to max to make it 100% opaque
    for (auto &pixel : *image) {
      pixel.a = 255u;
    }
    return image;
  }

} // namespace s11n
} // namespace sensor
} // namespace carla