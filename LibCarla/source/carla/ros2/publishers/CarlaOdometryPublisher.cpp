// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaOdometryPublisher.h"

#include "carla/ros2/publishers/UeToRosConversions.h"

namespace carla {
namespace ros2 {

bool CarlaOdometryPublisher::Write(
    int32_t seconds,
    uint32_t nanoseconds,
    std::string frame_id,
    std::string child_frame_id,
    const geom::Transform &transform,
    const geom::Vector3D &linear_velocity,
    const geom::Vector3D &angular_velocity) {
  auto message = _impl->GetMessage();

  message->header.stamp.sec = seconds;
  message->header.stamp.nanosec = nanoseconds;
  message->header.frame_id = frame_id;
  message->child_frame_id = child_frame_id;

  message->pose.pose.position.x = transform.location.x;
  message->pose.pose.position.y = -transform.location.y;
  message->pose.pose.position.z = transform.location.z;
  message->pose.pose.orientation = ue_rotation_to_ros_quaternion(transform.rotation);

  // Twist linear velocity is expressed in the child (vehicle) frame per the
  // nav_msgs/Odometry convention.
  message->twist.twist.linear =
      ue_world_velocity_to_ros_body_velocity(linear_velocity, transform.rotation);
  message->twist.twist.angular = ue_angular_velocity_to_ros(angular_velocity);

  return true;
}

}  // namespace ros2
}  // namespace carla
