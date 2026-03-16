#pragma once
#include "carla/ros2/types/msg/Vector3.h"
#include "carla/ros2/types/msg/Quaternion.h"

namespace carla {
namespace ros2 {
namespace msg {

struct Transform {
  Vector3 translation;
  Quaternion rotation;
};

} // namespace msg
} // namespace ros2
} // namespace carla
