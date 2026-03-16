#pragma once
#include <string>
#include "carla/ros2/types/msg/Time.h"

namespace carla {
namespace ros2 {
namespace msg {

struct Header {
  Time stamp;
  std::string frame_id;
};

} // namespace msg
} // namespace ros2
} // namespace carla
