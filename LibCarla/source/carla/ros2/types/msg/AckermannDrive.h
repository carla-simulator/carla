#pragma once

namespace carla {
namespace ros2 {
namespace msg {

struct AckermannDrive {
  float steering_angle = 0.0f;
  float steering_angle_velocity = 0.0f;
  float speed = 0.0f;
  float acceleration = 0.0f;
  float jerk = 0.0f;
};

} // namespace msg
} // namespace ros2
} // namespace carla
