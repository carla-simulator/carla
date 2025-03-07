// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <variant>
#include <functional>

namespace carla {
namespace ros2 {

  struct MultirotorControl
  {
    std::vector<float>   throttle;
  };

  using ROS2MultirotorCallbackData = std::variant<MultirotorControl>;

  using MultirotorActorCallback = std::function<void(void *actor, ROS2MultirotorCallbackData data)>;
  
} // namespace ros2
} // namespace carla
