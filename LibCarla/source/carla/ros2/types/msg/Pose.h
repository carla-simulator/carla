#pragma once
#include "carla/ros2/types/msg/Point.h"
#include "carla/ros2/types/msg/Quaternion.h"

namespace carla {
namespace ros2 {
namespace msg {

struct Pose {
  Point position;
  Quaternion orientation;
};

} // namespace msg
} // namespace ros2
} // namespace carla
