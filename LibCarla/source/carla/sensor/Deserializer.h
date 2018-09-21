// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Memory.h"

namespace carla {
namespace sensor {

  class SensorData;

  class Deserializer {
  public:

    static SharedPtr<SensorData> Deserialize(Buffer buffer);
  };

} // namespace sensor
} // namespace carla
