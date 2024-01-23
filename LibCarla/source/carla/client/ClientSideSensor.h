// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Sensor.h"

namespace carla {
namespace client {

  class ClientSideSensor : public Sensor {
  public:

    using Sensor::Sensor;
  };

} // namespace client
} // namespace carla
