// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaIMUPublisher.h"

namespace carla {
namespace ros2 {

bool CarlaIMUPublisher::Write(int32_t seconds, uint32_t nanoseconds, float* pAccelerometer, float* pGyroscope, float compass) {

  _impl->GetMessage()->header().stamp().sec(seconds);
  _impl->GetMessage()->header().stamp().nanosec(nanoseconds);
  _impl->GetMessage()->header().frame_id(GetFrameId());

  const float gx = *pGyroscope++;
  const float gy = *pGyroscope++;
  const float gz = *pGyroscope++;
  _impl->GetMessage()->angular_velocity().x(gx);
  _impl->GetMessage()->angular_velocity().y(gy);
  _impl->GetMessage()->angular_velocity().z(gz);

  const float ax = *pAccelerometer++;
  const float ay = *pAccelerometer++;
  const float az = *pAccelerometer++;
  _impl->GetMessage()->linear_acceleration().x(ax);
  _impl->GetMessage()->linear_acceleration().y(ay);
  _impl->GetMessage()->linear_acceleration().z(az);

  const float rx = 0.0f;                                 // pitch
  const float ry = (float(M_PI_2) / 2.0f) - compass;     // yaw
  const float rz = 0.0f;                                 // roll

  const float cr = cosf(rz * 0.5f);
  const float sr = sinf(rz * 0.5f);
  const float cp = cosf(rx * 0.5f);
  const float sp = sinf(rx * 0.5f);
  const float cy = cosf(ry * 0.5f);
  const float sy = sinf(ry * 0.5f);

  const float w = cr * cp * cy + sr * sp * sy;
  const float x = sr * cp * cy - cr * sp * sy;
  const float y = cr * sp * cy + sr * cp * sy;
  const float z = cr * cp * sy - sr * sp * cy;

  _impl->GetMessage()->orientation().w(w);
  _impl->GetMessage()->orientation().x(x);
  _impl->GetMessage()->orientation().y(y);
  _impl->GetMessage()->orientation().z(z);

  return true;
}

}  // namespace ros2
}  // namespace carla
