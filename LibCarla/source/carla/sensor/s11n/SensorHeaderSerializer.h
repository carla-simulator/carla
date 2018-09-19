// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/rpc/Transform.h"
#include "carla/sensor/SensorRegistry.h"

namespace carla {
namespace sensor {
namespace s11n {

  class SensorHeaderSerializer {
  public:

    template <typename Sensor>
    static Buffer Serialize(const Sensor &sensor, uint16_t frame_counter) {
      return Serialize(
          SensorRegistry::template get<Sensor*>::index,
          frame_counter,
          sensor.GetActorTransform());
    }

  private:

    static Buffer Serialize(uint64_t index, uint64_t frame, rpc::Transform transform);
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
