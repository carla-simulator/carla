#pragma once
#include <array>
#include "carla/ros2/types/msg/Twist.h"

namespace carla {
namespace ros2 {
namespace msg {

struct TwistWithCovariance {
  Twist twist;
  std::array<double, 36> covariance = {};
};

} // namespace msg
} // namespace ros2
} // namespace carla
