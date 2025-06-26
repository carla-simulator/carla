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

  struct CameraControl
  {
    float   pan;
    float   tilt;
    float   zoom;
  };

  using ROS2CameraControlCallbackData = std::variant<CameraControl>;

  using CameraControlActorCallback = std::function<void(void *actor, ROS2CameraControlCallbackData data)>;

} // namespace ros2
} // namespace carla
