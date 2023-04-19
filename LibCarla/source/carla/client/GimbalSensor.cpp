// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/GimbalSensor.h"

#include "carla/client/detail/Simulator.h"
#include "carla/Memory.h"

namespace carla {
namespace client {

  void GimbalSensor::SetMode(const GimbalMode mode) {
    GetEpisode().Lock()->SetGimbalMode(*this, mode);
  }

  void GimbalSensor::SetCmd(const float roll, const float pitch, const float yaw) {
    GetEpisode().Lock()->SetGimbalCmd(*this, roll, pitch, yaw);
  }

} // namespace client
} // namespace carla
