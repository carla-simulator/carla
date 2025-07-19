// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaTransformPublisher.h"

namespace carla {
namespace ros2 {

bool CarlaTransformPublisher::Write(int32_t seconds, uint32_t nanoseconds, std::string frame_id, std::string child_frame_id, const float* translation, const float* rotation) {
//   int same_translation = std::memcmp(translation, _impl->last_translation, sizeof(float) * 3);
//   int same_rotation = std::memcmp(rotation, _impl->last_rotation, sizeof(float) * 3);
//   if (same_translation != 0 || same_rotation != 0) {
//       std::memcpy(_impl->last_translation, translation, sizeof(float) * 3);
//       std::memcpy(_impl->last_rotation, rotation, sizeof(float) * 3);

//       const float tx = *translation++;
//       const float ty = *translation++;
//       const float tz = *translation++;

//       const float rx = ((*rotation++) * -1.0f) * (float(M_PI_2) / 180.0f);
//       const float ry = ((*rotation++) * -1.0f) * (float(M_PI_2) / 180.0f);
//       const float rz = *rotation++ * (float(M_PI_2) / 180.0f);

//       const float cr = cosf(rz * 0.5f);
//       const float sr = sinf(rz * 0.5f);
//       const float cp = cosf(rx * 0.5f);
//       const float sp = sinf(rx * 0.5f);
//       const float cy = cosf(ry * 0.5f);
//       const float sy = sinf(ry * 0.5f);

//       _impl->vec_translation.x(tx);
//       _impl->vec_translation.y(-ty);
//       _impl->vec_translation.z(tz);

//       _impl->vec_rotation.w(cr * cp * cy + sr * sp * sy);
//       _impl->vec_rotation.x(sr * cp * cy - cr * sp * sy);
//       _impl->vec_rotation.y(cr * sp * cy + sr * cp * sy);
//       _impl->vec_rotation.z(cr * cp * sy - sr * sp * cy);
//   }

//   _impl->GetMessage()->clock().sec(seconds);
//   _impl->GetMessage()->clock().nanosec(nanoseconds);

  // TODO: Check same translation, same rotation to avoid doing this every tick for every sensor.
  const float tx = *translation++;
  const float ty = *translation++;
  const float tz = *translation++;

  const float rx = ((*rotation++) * -1.0f) * (float(M_PI_2) / 180.0f);
  const float ry = ((*rotation++) * -1.0f) * (float(M_PI_2) / 180.0f);
  const float rz = *rotation++ * (float(M_PI_2) / 180.0f);

  const float cr = cosf(rz * 0.5f);
  const float sr = sinf(rz * 0.5f);
  const float cp = cosf(rx * 0.5f);
  const float sp = sinf(rx * 0.5f);
  const float cy = cosf(ry * 0.5f);
  const float sy = sinf(ry * 0.5f);

  builtin_interfaces::msg::Time time;
  time.sec(seconds);
  time.nanosec(nanoseconds);

  std_msgs::msg::Header header;
  header.stamp(std::move(time));
  header.frame_id(frame_id);

  geometry_msgs::msg::Vector3 translation_msg;
  translation_msg.x(tx);
  translation_msg.y(-ty);
  translation_msg.z(tz);

  geometry_msgs::msg::Quaternion quaternion_msg;
  quaternion_msg.w(cr * cp * cy + sr * sp * sy);
  quaternion_msg.x(sr * cp * cy - cr * sp * sy);
  quaternion_msg.y(cr * sp * cy + sr * cp * sy);
  quaternion_msg.z(cr * cp * sy - sr * sp * cy);

  geometry_msgs::msg::Transform t;
  t.rotation(quaternion_msg);
  t.translation(translation_msg);

  geometry_msgs::msg::TransformStamped ts;
  ts.header(std::move(header));
  ts.transform(std::move(t));
  ts.child_frame_id(child_frame_id);

  _impl->GetMessage()->transforms({ts});

  return true;
}

}  // namespace ros2
}  // namespace carla
