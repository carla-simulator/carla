// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ClockPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

ClockPublisher::ClockPublisher()
  : PublisherBase(carla::ros2::types::ActorNameDefinition::CreateFromRoleName("clock")),
    _impl(std::make_shared<ClockPublisherImpl>()) {}

bool ClockPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, "rt/clock", get_topic_qos());
}

bool ClockPublisher::Publish() {
  return _impl->Publish();
}

bool ClockPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void ClockPublisher::UpdateData(const builtin_interfaces::msg::Time &stamp) {
  _impl->Message().clock(stamp);
  _impl->SetMessageUpdated();
}
}  // namespace ros2
}  // namespace carla
