// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
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

  Considers the conversion from left-handed system (unreal) with axis x-forward, y-rightwards, z-up
  to right-handed system (ROS) with axis x-forward, y-leftwards, z-up. 
  If you were moving to a pure LH system where the Y-axis also pointed Left, you would negate everything.
  But Unreal flipped the axis direction (Left to Right) and flipped the handedness (RH to LH), 
  those two flips "cancel out" for the X and Y rotations.
  Therefore, only the z component of the quaternion is negated!
*/
class Quaternion {
public:
  /**
   * carla_rotation: the carla Rotation
   */
  explicit Quaternion(const geom::Quaternion& carla_quaternion) {
    // negate z component to convert from Unreal left-handed system to ROS right-handed system
    _ros_quaternion.x(carla_quaternion.x);
    _ros_quaternion.y(carla_quaternion.y);
    _ros_quaternion.z(-carla_quaternion.z);
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
    // negate z component to convert from Unreal left-handed system to ROS right-handed system
    carla_quaternion.x = float(_ros_quaternion.x());
    carla_quaternion.y = float(_ros_quaternion.y());
    carla_quaternion.z = float(-_ros_quaternion.z());
    carla_quaternion.w = float(_ros_quaternion.w());
    return carla_quaternion;
  }

private:
  geometry_msgs::msg::Quaternion _ros_quaternion;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla