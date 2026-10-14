// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "carla/ros2/types/msg/Accel.h"
#include "carla/ros2/types/msg/CarlaEgoVehicleControl.h"
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/Quaternion.h"

namespace carla {
namespace ros2 {
namespace msg {

struct CarlaEgoVehicleStatus {
  Header header;
  float velocity = 0.0f;
  Accel acceleration;
  Quaternion orientation;
  CarlaEgoVehicleControl control;
};

} // namespace msg
} // namespace ros2
} // namespace carla
