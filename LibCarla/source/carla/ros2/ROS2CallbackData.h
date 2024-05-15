// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <boost/variant2/variant.hpp>
#pragma warning(pop)
#else
#include <boost/variant2/variant.hpp>
#endif

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

    struct MessageControl
  {
    const char* message;
  };

  using ROS2CallbackData = boost::variant2::variant<VehicleControl>;
  using ROS2MessageCallbackData = boost::variant2::variant<MessageControl>;

  using ActorCallback = std::function<void(void *actor, ROS2CallbackData data)>;
  using ActorMessageCallback = std::function<void(void *actor, ROS2MessageCallbackData data)>;
  
} // namespace ros2
} // namespace carla
