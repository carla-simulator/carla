// Copyright (c) 2020 Robotics and Perception Group (GPR)
// University of Zurich and ETH Zurich
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/DVSEventArraySerializer.h"

#include "carla/sensor/data/DVSEventArray.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> DVSEventArraySerializer::Deserialize(RawData &&data) {

    auto events_array = SharedPtr<data::DVSEventArray>(new data::DVSEventArray{std::move(data)});

    return events_array;
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
