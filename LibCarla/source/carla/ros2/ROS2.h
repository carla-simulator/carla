// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "carla/Logging.h"
#include "carla/Buffer.h"
#include <functional>

namespace carla {
namespace ros2 {

class ROS2 : public std::enable_shared_from_this<ROS2>
{
  public:
  void set_frame(uint64_t frame);

  private:
  uint64_t _frame;
};

} // namespace ros2
} // namespace carla
