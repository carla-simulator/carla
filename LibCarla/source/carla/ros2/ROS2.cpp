// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/ROS2.h"

namespace carla {
namespace ros2 {

void ROS2::set_frame(uint64_t frame) { 
  _frame = frame; 
}

} // namespace ros2
} // namespace carla
