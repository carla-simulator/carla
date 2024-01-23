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

  struct VehicleControl
  {
    float   throttle;
    float   steer;
    float   brake;
    bool    hand_brake;
    bool    reverse;
    int32_t gear;
    bool    manual_gear_shift;
  };

  using ROS2CallbackData = std::variant<VehicleControl>;

  using ActorCallback = std::function<void(void *actor, ROS2CallbackData data)>;

} // namespace ros2
} // namespace carla
