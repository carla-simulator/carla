// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Math.h"
#include "carla/geom/Quaternion.h"
#include "geometry_msgs/msg/Quaternion.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Convert a carla rotation to a ROS quaternion

  Considers the conversion from left-handed system (unreal) to right-handed
  system (ROS).
  Considers the conversion from degrees (carla) to radians (ROS).
*/
class Quaternion {
public:
  /**
   * carla_rotation: the carla Rotation
   */
  explicit Quaternion(const geom::Quaternion& carla_quaternion) {
    // left-handed to right-handed -> negate the rotation by negating all axis components
    // switch y-axis from right to left -> negate y-axis
    _ros_quaternion.x(-carla_quaternion.x);  // -(forward =  forward)
    _ros_quaternion.y(carla_quaternion.y);   // -(  right = -left  )
    _ros_quaternion.z(-carla_quaternion.z);  // -(     up =  up     )
    _ros_quaternion.w(carla_quaternion.w);
  }
  /**
   * carla_rotation: the carla Rotation
   */
  explicit Quaternion(const geometry_msgs::msg::Quaternion& ros_quaternion) : _ros_quaternion(ros_quaternion) {}

  ~Quaternion() = default;
  Quaternion(const Quaternion&) = default;
  Quaternion& operator=(const Quaternion&) = default;
  Quaternion(Quaternion&&) = default;
  Quaternion& operator=(Quaternion&&) = default;

  /**
   * The resulting ROS geometry_msgs::msg::Quaternion
   */
  geometry_msgs::msg::Quaternion quaternion() const {
    return _ros_quaternion;
  }

  geom::Quaternion GetQuaternion() const {
    geom::Quaternion carla_quaternion;
    // left-handed to right-handed -> negate the rotation by negating all axis components
    // switch y-axis from right to left -> negate y-axis
    carla_quaternion.x = float(-_ros_quaternion.x());  // -(forward =  forward)
    carla_quaternion.y = float(_ros_quaternion.y());   // -(  right = -left  )
    carla_quaternion.z = float(-_ros_quaternion.z());  // -(     up =  up     )
    carla_quaternion.w = float(_ros_quaternion.w());
    return carla_quaternion;
  }

private:
  geometry_msgs::msg::Quaternion _ros_quaternion;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla