#pragma once
#include <vector>
#include "carla/ros2/types/msg/TransformStamped.h"

namespace carla {
namespace ros2 {
namespace msg {

struct TFMessage {
  std::vector<TransformStamped> transforms;
};

} // namespace msg
} // namespace ros2
} // namespace carla
