#include "carla/sensor/s11n/GimbalSerializer.h"

#include "carla/sensor/data/GimbalEvent.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> GimbalSerializer::Deserialize(RawData &&data) {
    return SharedPtr<SensorData>(new data::GimbalEvent(std::move(data)));
  }

} // namespace s11n
} // namespace sensor
} // namespace carla