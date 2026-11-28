// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaOdometryPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/publishers/ImuMath.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/publishers/TransformQuaternion.h"
#include "carla/ros2/types/msg/Odometry.h"

namespace carla {
namespace ros2 {

struct CarlaOdometryMsgTraits {
  using msg_type = msg::Odometry;
};

CarlaOdometryPublisher::CarlaOdometryPublisher(
    std::string base_topic_name, std::string frame_id)
  : BasePublisher(std::move(base_topic_name), std::move(frame_id)),
    _impl(std::make_shared<PublisherImpl<CarlaOdometryMsgTraits>>()) {
  if (!_impl->Init(GetBaseTopicName() + "/odometry")) {
    log_error("CarlaOdometryPublisher: failed to initialise writer for", GetBaseTopicName() + "/odometry");
  }
}

CarlaOdometryPublisher::~CarlaOdometryPublisher() = default;

bool CarlaOdometryPublisher::Publish() {
  return _impl->Publish();
}

bool CarlaOdometryPublisher::Write(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    float x, float y, float z,
    float pitch_deg, float yaw_deg, float roll_deg,
    float vx, float vy, float vz,
    float wx, float wy, float wz) {
  auto *message = _impl->GetMessage();
  message->header.stamp.sec = seconds;
  message->header.stamp.nanosec = nanoseconds;
  message->header.frame_id = "odom";
  message->child_frame_id = GetFrameId();

  const auto pose = TransformFromCarlaRotation(x, y, z, pitch_deg, yaw_deg, roll_deg);
  message->pose.pose.position.x = pose.translation[0];
  message->pose.pose.position.y = pose.translation[1];
  message->pose.pose.position.z = pose.translation[2];
  message->pose.pose.orientation.w = pose.rotation[0];
  message->pose.pose.orientation.x = pose.rotation[1];
  message->pose.pose.orientation.y = pose.rotation[2];
  message->pose.pose.orientation.z = pose.rotation[3];

  // Linear velocity is a vector and angular velocity a pseudovector, so they
  // flip like the IMU's accelerometer and gyroscope.
  const auto linear = AccelToRos(vx, vy, vz);
  message->twist.twist.linear.x = linear[0];
  message->twist.twist.linear.y = linear[1];
  message->twist.twist.linear.z = linear[2];

  const auto angular = GyroToRos(wx, wy, wz);
  message->twist.twist.angular.x = angular[0];
  message->twist.twist.angular.y = angular[1];
  message->twist.twist.angular.z = angular[2];

  return true;
}

}  // namespace ros2
}  // namespace carla
