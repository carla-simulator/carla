// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaTransformPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/publishers/TransformQuaternion.h"
#include "carla/ros2/types/TFMessage.h"
#include "carla/ros2/types/TFMessagePubSubTypes.h"

#include <cmath>

namespace carla {
namespace ros2 {

struct CarlaTransformMsgTraits {
  using msg_type = tf2_msgs::msg::TFMessage;
  using msg_pubsub_type = tf2_msgs::msg::TFMessagePubSubType;
};

namespace {

constexpr float kEpsilon = 1e-4f;

bool IsSameTransform(
    const CarlaTransformPublisher::CachedTransform &cached,
    float tx, float ty, float tz,
    float pitch, float yaw, float roll) noexcept {
  return std::abs(cached.tx - tx) < kEpsilon
      && std::abs(cached.ty - ty) < kEpsilon
      && std::abs(cached.tz - tz) < kEpsilon
      && std::abs(cached.pitch - pitch) < kEpsilon
      && std::abs(cached.yaw - yaw) < kEpsilon
      && std::abs(cached.roll - roll) < kEpsilon;
}

}  // namespace

CarlaTransformPublisher::CarlaTransformPublisher()
  : BasePublisher("rt/tf"),
    _impl(std::make_shared<PublisherImpl<CarlaTransformMsgTraits>>()) {
  if (!_impl->Init(GetBaseTopicName())) {
    log_error("CarlaTransformPublisher: failed to initialise writer for rt/tf");
  }
}

CarlaTransformPublisher::~CarlaTransformPublisher() = default;

bool CarlaTransformPublisher::Publish() {
  return _impl->Publish();
}

bool CarlaTransformPublisher::Write(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    const std::string &parent_frame_id,
    const std::string &child_frame_id,
    float tx, float ty, float tz,
    float pitch_deg, float yaw_deg, float roll_deg) {
  std::array<float, 3> translation;
  std::array<float, 4> rotation;

  auto it = _last_transforms.find(child_frame_id);
  if (it != _last_transforms.end()
      && IsSameTransform(it->second, tx, ty, tz, pitch_deg, yaw_deg, roll_deg)) {
    translation = it->second.translation;
    rotation = it->second.rotation;
  } else {
    // Quaternion conversion lives in TransformQuaternion.h (header-only
    // seam, pinned by test_transform_quaternion.cpp).
    const auto computed =
        TransformFromCarlaRotation(tx, ty, tz, pitch_deg, yaw_deg, roll_deg);
    translation = computed.translation;
    rotation = computed.rotation;
    _last_transforms.insert_or_assign(
        child_frame_id,
        CachedTransform{
            tx, ty, tz, pitch_deg, yaw_deg, roll_deg, translation, rotation});
  }

  geometry_msgs::msg::TransformStamped ts;
  ts.header().stamp().sec(seconds);
  ts.header().stamp().nanosec(nanoseconds);
  ts.header().frame_id(parent_frame_id);
  ts.child_frame_id(child_frame_id);

  ts.transform().translation().x(translation[0]);
  ts.transform().translation().y(translation[1]);
  ts.transform().translation().z(translation[2]);
  ts.transform().rotation().w(rotation[0]);
  ts.transform().rotation().x(rotation[1]);
  ts.transform().rotation().y(rotation[2]);
  ts.transform().rotation().z(rotation[3]);

  _impl->GetMessage()->transforms({ts});
  return true;
}

}  // namespace ros2
}  // namespace carla
