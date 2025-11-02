// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaTransformPublisher.h"

namespace carla {
namespace ros2 {

constexpr double EPSILON = 1e-4;

geometry_msgs::msg::Transform CarlaTransformPublisher::ComputeTransform(std::string frame_id, geom::Transform transform) {

  // Avoid recomputing the transform if it hasn't changed.
  // This is common for static sensors that are typically attached to other actors.
  auto it = _last_transforms.find(frame_id);
  if (it != _last_transforms.end()) {
    const auto& [last_transform, last_tf] = it->second;

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

  // Better readability
  const float tx = transform.location.x;
  const float ty = transform.location.y * -1.0f;
  const float tz = transform.location.z;

  // Rotations was not correctly computed Radians = Degrees * (π / 180)
  const float DEG_TO_RAD = float(M_PI) / 180.0f;
  const float rx = (transform.rotation.pitch * -1.0f) * DEG_TO_RAD;
  const float ry = (transform.rotation.yaw * -1.0f) * DEG_TO_RAD;
  const float rz = transform.rotation.roll * DEG_TO_RAD;

  const float cr = cosf(rz * 0.5f);
  const float sr = sinf(rz * 0.5f);
  const float cp = cosf(rx * 0.5f);
  const float sp = sinf(rx * 0.5f);
  const float cy = cosf(ry * 0.5f);
  const float sy = sinf(ry * 0.5f);

  geometry_msgs::msg::Transform tf;

  tf.translation().x(tx);
  tf.translation().y(ty);
  tf.translation().z(tz);

  tf.rotation().w(cr * cp * cy + sr * sp * sy);
  tf.rotation().x(sr * cp * cy - cr * sp * sy);
  tf.rotation().y(cr * sp * cy + sr * cp * sy);
  tf.rotation().z(cr * cp * sy - sr * sp * cy);

  return tf;
}

bool CarlaTransformPublisher::Write(int32_t seconds, uint32_t nanoseconds, std::string frame_id, std::string child_frame_id, geom::Transform transform) {


  geometry_msgs::msg::TransformStamped ts;

  ts.header().stamp().sec(seconds);
  ts.header().stamp().nanosec(nanoseconds);
  ts.header().frame_id(frame_id);

  auto tf = ComputeTransform(child_frame_id, transform);
  ts.transform(tf);

  ts.child_frame_id(child_frame_id);

  _impl->GetMessage()->transforms({ts});

  // Update last transform information
  _last_transforms.insert({child_frame_id, {transform, tf}});

  return true;
}

}  // namespace ros2
}  // namespace carla
