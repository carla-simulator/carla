#pragma once
#include <array>
#include "carla/ros2/types/msg/Pose.h"

namespace carla {
namespace ros2 {
namespace msg {

struct PoseWithCovariance {
  Pose pose;
  std::array<double, 36> covariance = {};
};

} // namespace msg
} // namespace ros2
} // namespace carla
