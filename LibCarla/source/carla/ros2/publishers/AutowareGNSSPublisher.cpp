// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/AutowareGNSSPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/middleware/QosProfile.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/publishers/TransformQuaternion.h"
#include "carla/ros2/types/msg/Pose.h"
#include "carla/ros2/types/msg/PoseWithCovarianceStamped.h"

namespace carla {
namespace ros2 {

struct AutowareGnssPoseTraits { using msg_type = msg::Pose; };
struct AutowareGnssPoseWithCovarianceTraits {
  using msg_type = msg::PoseWithCovarianceStamped;
};

class AutowareGNSSPublisher::Impl {
public:
  PublisherImpl<AutowareGnssPoseTraits> pose;
  PublisherImpl<AutowareGnssPoseWithCovarianceTraits> pose_with_covariance;
};

AutowareGNSSPublisher::AutowareGNSSPublisher(
    std::string base_topic_name, std::string frame_id)
  : BasePublisher(std::move(base_topic_name), std::move(frame_id)),
    _impl(std::make_shared<Impl>()) {
  // QoS per tier4's AutowareGNSSPublisher: RELIABLE / VOLATILE / KEEP_LAST 1.
  const QosProfile qos = QosProfile::ReliableVolatile();
  bool ok = true;
  ok &= _impl->pose.Init(GetBaseTopicName() + "/pose", qos);
  ok &= _impl->pose_with_covariance.Init(
      GetBaseTopicName() + "/pose_with_covariance", qos);
  if (!ok) {
    log_error("AutowareGNSSPublisher: failed to initialise writers for",
              GetBaseTopicName());
  }
}

AutowareGNSSPublisher::~AutowareGNSSPublisher() = default;

bool AutowareGNSSPublisher::Write(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    float tx, float ty, float tz,
    float pitch_deg, float yaw_deg, float roll_deg,
    double mgrs_offset_x, double mgrs_offset_y, double mgrs_offset_z) {
  // Same CARLA-to-ROS handedness conversion as CarlaTransformPublisher
  // (tier4 copied that math into AutowareGNSSPublisher::SetData).
  const TransformQuaternion tq =
      TransformFromCarlaRotation(tx, ty, tz, pitch_deg, yaw_deg, roll_deg);

  auto *pose = _impl->pose.GetMessage();
  pose->position.x = static_cast<double>(tq.translation[0]) + mgrs_offset_x;
  pose->position.y = static_cast<double>(tq.translation[1]) + mgrs_offset_y;
  pose->position.z = static_cast<double>(tq.translation[2]) + mgrs_offset_z;
  pose->orientation.w = tq.rotation[0];
  pose->orientation.x = tq.rotation[1];
  pose->orientation.y = tq.rotation[2];
  pose->orientation.z = tq.rotation[3];

  auto *pwcs = _impl->pose_with_covariance.GetMessage();
  pwcs->header.stamp.sec = seconds;
  pwcs->header.stamp.nanosec = nanoseconds;
  pwcs->header.frame_id = GetFrameId();
  pwcs->pose.pose = *pose;
  pwcs->pose.covariance = {};  // TODO(tier4 parity): meaningful covariance.
  return true;
}

bool AutowareGNSSPublisher::Publish() {
  bool ok = true;
  ok &= _impl->pose.Publish();
  ok &= _impl->pose_with_covariance.Publish();
  return ok;
}

}  // namespace ros2
}  // namespace carla
