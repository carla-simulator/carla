// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"
#include "carla/ros2/types/AngularVelocity.h"
#include "carla/ros2/types/Speed.h"
#include "geometry_msgs/msg/Twist.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Convert carla velocities to a ROS twist

  Considers the conversion from left-handed system (unreal) to right-handed
  system (ROS).
*/
class Twist {
public:
  /**
   * carla_Twist: the carla Twist
   */
  Twist(Speed const& speed, AngularVelocity const& angular_velocity) {
    _ros_twist.linear().x(speed.LinearVelocityROS().x);
    _ros_twist.linear().y(speed.LinearVelocityROS().y);
    _ros_twist.linear().z(speed.LinearVelocityROS().z);
    _ros_twist.angular().x(angular_velocity.AngularVelocityROS().x);
    _ros_twist.angular().y(angular_velocity.AngularVelocityROS().y);
    _ros_twist.angular().z(angular_velocity.AngularVelocityROS().z);
  }
  ~Twist() = default;
  Twist(const Twist&) = default;
  Twist& operator=(const Twist&) = default;
  Twist(Twist&&) = default;
  Twist& operator=(Twist&&) = default;

  /**
   * The resulting ROS geometry_msgs::msg::twist
   */
  geometry_msgs::msg::Twist twist() const {
    return _ros_twist;
  }

private:
  geometry_msgs::msg::Twist _ros_twist;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla