#pragma once
#include <string>
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/Transform.h"

namespace carla {
namespace ros2 {
namespace msg {

struct TransformStamped {
  Header header;
  std::string child_frame_id;
  Transform transform;
};

} // namespace msg
} // namespace ros2
} // namespace carla
