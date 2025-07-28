// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaTransformPublisher.h"

namespace carla {
namespace ros2 {

bool CarlaTransformPublisher::Write(int32_t seconds, uint32_t nanoseconds, std::string frame_id, std::string child_frame_id, geom::Transform transform) {
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
  const float tx = transform.location.x;
  const float ty = transform.location.y;
  const float tz = transform.location.z;

  const float rx = (transform.rotation.pitch * -1.0f) * (float(M_PI_2) / 180.0f);
  const float ry = (transform.rotation.yaw * -1.0f) * (float(M_PI_2) / 180.0f);
  const float rz = transform.rotation.roll * (float(M_PI_2) / 180.0f);

  const float cr = cosf(rz * 0.5f);
  const float sr = sinf(rz * 0.5f);
  const float cp = cosf(rx * 0.5f);
  const float sp = sinf(rx * 0.5f);
  const float cy = cosf(ry * 0.5f);
  const float sy = sinf(ry * 0.5f);

  geometry_msgs::msg::TransformStamped ts;

  ts.header().stamp().sec(seconds);
  ts.header().stamp().nanosec(nanoseconds);
  ts.header().frame_id(frame_id);

  ts.transform().translation().x(tx);
  ts.transform().translation().y(-ty);
  ts.transform().translation().z(tz);

  ts.transform().rotation().w(cr * cp * cy + sr * sp * sy);
  ts.transform().rotation().x(sr * cp * cy - cr * sp * sy);
  ts.transform().rotation().y(cr * sp * cy + sr * cp * sy);
  ts.transform().rotation().z(cr * cp * sy - sr * sp * cy);

  ts.child_frame_id(child_frame_id);

  _impl->GetMessage()->transforms({ts});

  return true;
}

}  // namespace ros2
}  // namespace carla
