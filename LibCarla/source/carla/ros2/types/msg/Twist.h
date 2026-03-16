#pragma once
#include "carla/ros2/types/msg/Vector3.h"

namespace carla {
namespace ros2 {
namespace msg {

struct Twist {
  Vector3 linear;
  Vector3 angular;
};

} // namespace msg
} // namespace ros2
} // namespace carla
