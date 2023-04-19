// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Sensor.h"
#include "carla/rpc/GimbalMode.h"

namespace carla {

namespace client {

  class GimbalSensor : public Sensor {
  public:
    // using statements here
    using GimbalMode = rpc::GimbalMode;

    // explicit GimbalSensor(ActorInitializer init);

    /// Set Gimbal commands for roll, pitch, and yaw
    void SetCmd(const float roll, const float pitch, const float yaw);

    /// Set Gimbal commands for roll, pitch, and yaw
    void SetMode(const GimbalMode mode);
  };

} // namespace client
} // namespace carla
