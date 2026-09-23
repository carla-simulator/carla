// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <array>
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/Quaternion.h"
#include "carla/ros2/types/msg/Vector3.h"

namespace carla {
namespace ros2 {
namespace msg {

struct Imu {
  Header header;
  Quaternion orientation;
  std::array<double, 9> orientation_covariance = {};
  Vector3 angular_velocity;
  std::array<double, 9> angular_velocity_covariance = {};
  Vector3 linear_acceleration;
  std::array<double, 9> linear_acceleration_covariance = {};
};

} // namespace msg
} // namespace ros2
} // namespace carla
