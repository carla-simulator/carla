// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/ImageSerializer.h"

#include "carla/sensor/data/Image.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> ImageSerializer::Deserialize(RawData data) {
    auto pixels = data.data();
    // Set alpha of each pixel in the buffer to max to make it 100% opaque
    for (size_t i = header_offset + 3; i < data.size(); i += 4) {
      pixels[i] = 255;
    }
    return SharedPtr<data::Image>(new data::Image{std::move(data)});
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
