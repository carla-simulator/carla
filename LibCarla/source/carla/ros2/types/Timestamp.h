// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cmath>

#include "builtin_interfaces/msg/Time.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Convert a carla Timestamp to a ROS builtin_interfaces::msg::Time
  and holds carla time details
*/
class Timestamp {
public:
  explicit Timestamp(float timestamp = 0.f) : _stamp(timestamp) {
    float integral;
    const float fractional = std::modf(timestamp, &integral);
    _ros_time.sec(static_cast<int32_t>(integral));
    _ros_time.nanosec(static_cast<uint32_t>(fractional * 1e9));
  }

  explicit Timestamp(double timestamp) : Timestamp(float(timestamp)) {}

  explicit Timestamp(const builtin_interfaces::msg::Time& time) : _ros_time(time) {
    _stamp = float(_ros_time.sec()) + 1e-9f * float(_ros_time.nanosec());
  }

  ~Timestamp() = default;
  Timestamp(const Timestamp&) = default;
  Timestamp& operator=(const Timestamp&) = default;
  Timestamp(Timestamp&&) = default;
  Timestamp& operator=(Timestamp&&) = default;

  float Stamp() const {
    return _stamp;
  }

  /**
   * The resulting ROS builtin_interfaces::msg::Time
   */
  const builtin_interfaces::msg::Time& time() const {
    return _ros_time;
  }

private:
  float _stamp{0.};
  builtin_interfaces::msg::Time _ros_time;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla