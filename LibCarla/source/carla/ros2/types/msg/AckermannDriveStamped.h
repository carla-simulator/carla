#pragma once
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/AckermannDrive.h"

namespace carla {
namespace ros2 {
namespace msg {

struct AckermannDriveStamped {
  Header header;
  AckermannDrive drive;
};

} // namespace msg
} // namespace ros2
} // namespace carla
