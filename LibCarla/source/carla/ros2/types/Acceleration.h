// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Acceleration.h"
#include "carla/ros2/types/CoordinateSystemTransform.h"
#include "geometry_msgs/msg/Accel.h"
#include "carla/geom/Quaternion.h"

namespace carla {
namespace ros2 {
namespace types {

class AcceleratedMovement;

/**
  Convert a carla (linear) acceleration to a ROS accel (linear part)

  Considers the conversion from left-handed system (unreal) to right-handed
  system (ROS)
*/
class Acceleration {
public:
  /**
   * carla_linear_acceleration: the carla linear acceleration; this is not provided by UE4
   * therefore has to be deduced from the Velocity
   */
  Acceleration(carla::geom::Acceleration const& carla_linear_acceleration = carla::geom::Acceleration()) {
    _carla_linear_acceleration = carla_linear_acceleration;
  }
  ~Acceleration() = default;
  Acceleration(const Acceleration&) = default;
  Acceleration& operator=(const Acceleration&) = default;
  Acceleration(Acceleration&&) = default;
  Acceleration& operator=(Acceleration&&) = default;


  geometry_msgs::msg::Vector3 linear_acceleration() const {
    return CoordinateSystemTransform::TransformLinearAxisMsg(_carla_linear_acceleration);
  }

  /**
   * The resulting ROS angular acceleration as geometry_msgs::msg::Vector3 in ROS coordinates
   */
  geometry_msgs::msg::Vector3 angular_acceleration() const {
    geometry_msgs::msg::Vector3 angular_acceleration_ros;
    angular_acceleration_ros.x() = -_carla_angular_acceleration_rad.x;  // -(forward =  forward)
    angular_acceleration_ros.y() = _carla_angular_acceleration_rad.y;  // -(  right = -left  )
    angular_acceleration_ros.z() = -_carla_angular_acceleration_rad.z;  // -(     up =  up     )
    return angular_acceleration_ros;
  }

  /**
   * The resulting ROS geometry_msgs::msg::Accel
   */
  geometry_msgs::msg::Accel accel() const {
    geometry_msgs::msg::Accel ros_accel;
    ros_accel.linear() = linear_acceleration();
    ros_accel.angular() = angular_acceleration();
    return ros_accel;
  }

    /**
   * Get the relative acceleration in the reference frame of the provided transform
   */
  Acceleration GetRelative(carla::geom::Quaternion const& quat) const {
    Acceleration relative_acceleration;
    relative_acceleration._carla_linear_acceleration = quat.InverseRotatedVector(_carla_linear_acceleration);
    relative_acceleration._carla_angular_acceleration_rad = quat.InverseRotatedVector(_carla_angular_acceleration_rad);
    return relative_acceleration;
  }

private:
  friend class AcceleratedMovement;

  carla::geom::Acceleration _carla_linear_acceleration;
  carla::geom::Acceleration _carla_angular_acceleration_rad;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla