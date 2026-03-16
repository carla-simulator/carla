#pragma once
#include <string>
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/PoseWithCovariance.h"
#include "carla/ros2/types/msg/TwistWithCovariance.h"

namespace carla {
namespace ros2 {
namespace msg {

struct Odometry {
  Header header;
  std::string child_frame_id;
  PoseWithCovariance pose;
  TwistWithCovariance twist;
};

} // namespace msg
} // namespace ros2
} // namespace carla
