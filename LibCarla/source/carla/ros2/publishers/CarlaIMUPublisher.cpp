// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaIMUPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/publishers/ImuMath.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/types/msg/Imu.h"

namespace carla {
namespace ros2 {

struct CarlaImuMsgTraits {
  using msg_type = msg::Imu;
};

CarlaIMUPublisher::CarlaIMUPublisher(
    std::string base_topic_name, std::string frame_id)
  : BasePublisher(std::move(base_topic_name), std::move(frame_id)),
    _impl(std::make_shared<PublisherImpl<CarlaImuMsgTraits>>()) {
  if (!_impl->Init(GetBaseTopicName())) {
    log_error("CarlaIMUPublisher: failed to initialise writer for", GetBaseTopicName());
  }
}

CarlaIMUPublisher::~CarlaIMUPublisher() = default;

bool CarlaIMUPublisher::Publish() {
  return _impl->Publish();
}

bool CarlaIMUPublisher::Write(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    float accel_x, float accel_y, float accel_z,
    float gyro_x, float gyro_y, float gyro_z,
    float compass) {
  auto *message = _impl->GetMessage();
  message->header.stamp.sec = seconds;
  message->header.stamp.nanosec = nanoseconds;
  message->header.frame_id = GetFrameId();

  // Input samples arrive in CARLA's left-handed sensor frame; convert to the
  // REP-103 right-handed frame before publishing (see ImuMath.h for the
  // pseudovector rationale on the gyro axes).
  const auto accel = AccelToRos(accel_x, accel_y, accel_z);
  message->linear_acceleration.x = accel[0];
  message->linear_acceleration.y = accel[1];
  message->linear_acceleration.z = accel[2];

  const auto gyro = GyroToRos(gyro_x, gyro_y, gyro_z);
  message->angular_velocity.x = gyro[0];
  message->angular_velocity.y = gyro[1];
  message->angular_velocity.z = gyro[2];

  // Yaw-only quaternion from compass heading; math lives in ImuMath.h.
  const auto q = OrientationFromCompass(compass);
  message->orientation.w = q[0];
  message->orientation.x = q[1];
  message->orientation.y = q[2];
  message->orientation.z = q[3];

  return true;
}

}  // namespace ros2
}  // namespace carla
