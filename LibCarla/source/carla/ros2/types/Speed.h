// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Math.h"
#include "carla/geom/Quaternion.h"
#include "carla/geom/Velocity.h"
#include "std_msgs/msg/Float32.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Convert a carla linear Speed to a ROS accel

  Considers the conversion from left-handed system (unreal) to right-handed
  system (ROS)

*/
class Speed {
public:
  /**
   * carla_speed: the carla linear Speed
   */
  Speed(carla::geom::Velocity const &carla_linear_velocity, carla::geom::Quaternion const &carla_quat) {
    _linear_velocity_ros.x = carla_linear_velocity.x;
    _linear_velocity_ros.y = -carla_linear_velocity.y;
    _linear_velocity_ros.z = carla_linear_velocity.z;
    _ros_speed.data(_linear_velocity_ros.Speed(carla_quat));
  }
#ifdef LIBCARLA_INCLUDED_FROM_UE4
  Speed(const FVector &carla_linear_velocity, const FQuat &carla_quat)
    : Speed(carla::geom::Velocity(carla_linear_velocity), carla::geom::Quaternion(carla_quat)) {}
#endif  // LIBCARLA_INCLUDED_FROM_UE4
  Speed() = default;
  ~Speed() = default;
  Speed(const Speed &) = default;
  Speed &operator=(const Speed &) = default;
  Speed(Speed &&) = default;
  Speed &operator=(Speed &&) = default;

  /**
   * The resulting ROS std_msgs::msg::Float32
   */
  std_msgs::msg::Float32 speed() const {
    return _ros_speed;
  }

  /**
   * The linear velocity as carla::geom::Vector3D but in ROS coordinates
   */
  carla::geom::Velocity LinearVelocityROS() const {
    return _linear_velocity_ros;
  }

private:
  carla::geom::Velocity _linear_velocity_ros;
  std_msgs::msg::Float32 _ros_speed;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla