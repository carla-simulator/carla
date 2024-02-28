//
// Created by flo on 09.11.20.
//

#include "NormalsImageSerializer.h"
#include "carla/sensor/s11n/NormalsImageSerializer.h"

#include "carla/sensor/data/Image.h"

namespace carla {
  namespace sensor {
    namespace s11n {

      SharedPtr<SensorData> NormalsImageSerializer::Deserialize(RawData DESERIALIZE_DECL_DATA(data)) {
        auto image = SharedPtr<data::NormalsImage>(new data::NormalsImage{DESERIALIZE_MOVE_DATA(data)});
        return image;
      }

    } // namespace s11n
  } // namespace sensor
} // namespace carla
