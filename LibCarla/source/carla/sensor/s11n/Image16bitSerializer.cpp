//
// Created by flo on 09.11.20.
//

#include "Image16bitSerializer.h"
#include "carla/sensor/s11n/Image16bitSerializer.h"

#include "carla/sensor/data/Image.h"

namespace carla {
  namespace sensor {
    namespace s11n {

      SharedPtr<SensorData> Image16bitSerializer::Deserialize(RawData &&data) {
        auto image = SharedPtr<data::Image16bit>(new data::Image16bit{std::move(data)});
        return image;
      }

    } // namespace s11n
  } // namespace sensor
} // namespace carla
