// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <vector>
#include "carla/ros2/types/msg/CarlaTrafficLightInfo.h"

namespace carla {
namespace ros2 {
namespace msg {

struct CarlaTrafficLightInfoList {
  std::vector<CarlaTrafficLightInfo> traffic_lights;
};

} // namespace msg
} // namespace ros2
} // namespace carla
