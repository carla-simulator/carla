// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#define _USE_MATH_DEFINES
#include <cmath>

#include "TransformPublisher.h"

#include "carla/ros2/impl/fastdds/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

TransformPublisher::TransformPublisher()
  : PublisherBase(carla::ros2::types::ActorNameDefinition::CreateFromRoleName("tf")),
    _impl(std::make_shared<TransformPublisherImpl>()) {}

bool TransformPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, "rt/tf", get_topic_qos());
}

bool TransformPublisher::Publish() {
  auto const success = _impl->Publish();
  if (success) {
    _impl->Message().transforms().clear();
  }
  return success;
}
bool TransformPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void TransformPublisher::AddTransform(const builtin_interfaces::msg::Time &stamp, std::string name, std::string parent,
                                      geometry_msgs::msg::Transform const &transform) {
  geometry_msgs::msg::TransformStamped ts;
  ts.header().stamp(stamp);
  ts.header().frame_id(parent);
  ts.transform(transform);
  ts.child_frame_id(name);
  _impl->Message().transforms().push_back(ts);
  _impl->SetMessageUpdated();
}
}  // namespace ros2
}  // namespace carla
