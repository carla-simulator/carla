// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Acceleration.h"
#include "carla/ros2/types/CoordinateSystemTransform.h"
#include "geometry_msgs/msg/Accel.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Convert a carla (linear) acceleration to a ROS accel (linear part)

  Considers the conversion from left-handed system (unreal) to right-handed
  system (ROS)
*/
class Acceleration {
public:
  /**
   * carla_acceleration: the carla linear acceleration; this is not provided by UE4
   * therefore has to be deduced from the Velocity
   */
  Acceleration(carla::geom::Acceleration const& carla_linear_acceleration = carla::geom::Acceleration()) {
    _ros_accel.linear() = CoordinateSystemTransform::TransformLinearAxisMsg(carla_linear_acceleration);
  }
  ~Acceleration() = default;
  Acceleration(const Acceleration&) = default;
  Acceleration& operator=(const Acceleration&) = default;
  Acceleration(Acceleration&&) = default;
  Acceleration& operator=(Acceleration&&) = default;

  /**
   * The resulting ROS geometry_msgs::msg::Accel
   */
  geometry_msgs::msg::Accel accel() const {
    return _ros_accel;
  }

private:
  geometry_msgs::msg::Accel _ros_accel;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla