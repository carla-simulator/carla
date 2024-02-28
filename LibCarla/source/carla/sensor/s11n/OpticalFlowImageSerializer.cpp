//
// Created by flo on 09.11.20.
//

#include "OpticalFlowImageSerializer.h"
#include "carla/sensor/s11n/OpticalFlowImageSerializer.h"

#include "carla/sensor/data/Image.h"

namespace carla {
  namespace sensor {
    namespace s11n {

      SharedPtr<SensorData> OpticalFlowImageSerializer::Deserialize(RawData DESERIALIZE_DECL_DATA(data)) {
        auto image = SharedPtr<data::OpticalFlowImage>(new data::OpticalFlowImage{DESERIALIZE_MOVE_DATA(data)});
        return image;
      }

    } // namespace s11n
  } // namespace sensor
} // namespace carla
