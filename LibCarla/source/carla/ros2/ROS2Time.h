// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/types/msg/Time.h"

#include <cmath>
#include <cstdint>

namespace carla {
namespace ros2 {

[[nodiscard]] inline msg::Time ToRosTime(double seconds) {
  constexpr double kNanosecondsPerSecond = 1000000000.0;
  double integral = 0.0;
  const double fractional = std::modf(seconds, &integral);
  msg::Time stamp;
  stamp.sec = static_cast<std::int32_t>(integral);
  stamp.nanosec = static_cast<std::uint32_t>(fractional * kNanosecondsPerSecond);
  return stamp;
}

} // namespace ros2
} // namespace carla
