// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaTransformPublisher.h"

#include "carla/ros2/publishers/UeToRosConversions.h"

namespace carla {
namespace ros2 {

constexpr double EPSILON = 1e-4;

msg::Transform CarlaTransformPublisher::ComputeTransform(std::string frame_id, geom::Transform transform) {

  // Avoid recomputing the transform if it hasn't changed.
  // This is common for static sensors that are typically attached to other actors.
  auto it = _last_transforms.find(frame_id);
  if (it != _last_transforms.end()) {
    const auto& last_transform = it->second.first;
    const auto& last_tf = it->second.second;

    // Do not use operator== directly on transforms.
    // Floating-point errors can cause two transforms that are equal to compare as different.
    if (std::abs(last_transform.location.x - transform.location.x) < EPSILON
      && std::abs(last_transform.location.y - transform.location.y) < EPSILON
      && std::abs(last_transform.location.z - transform.location.z) < EPSILON
      && std::abs(last_transform.rotation.roll - transform.rotation.roll) < EPSILON
      && std::abs(last_transform.rotation.pitch - transform.rotation.pitch) < EPSILON
      && std::abs(last_transform.rotation.yaw - transform.rotation.yaw) < EPSILON
    ) {
      return last_tf;
    }
  }

  msg::Transform tf;
  tf.translation = ue_vector_to_ros_vector(transform.location);
  tf.rotation = ue_rotation_to_ros_quaternion(transform.rotation);
  return tf;
}

bool CarlaTransformPublisher::Write(int32_t seconds, uint32_t nanoseconds, std::string frame_id, std::string child_frame_id, geom::Transform transform) {

  msg::TransformStamped ts;

  ts.header.stamp.sec = seconds;
  ts.header.stamp.nanosec = nanoseconds;
  ts.header.frame_id = frame_id;

  auto tf = ComputeTransform(child_frame_id, transform);
  ts.transform = tf;

  ts.child_frame_id = child_frame_id;

  _impl->GetMessage()->transforms = {ts};

  // Update last transform information
  _last_transforms.insert({child_frame_id, {transform, tf}});

  return true;
}

}  // namespace ros2
}  // namespace carla
