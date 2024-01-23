// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/NoopSerializer.h"

#include "carla/Exception.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> NoopSerializer::Deserialize(RawData &&) {
    throw_exception(std::runtime_error("NoopSerializer: Invalid data received."));
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
