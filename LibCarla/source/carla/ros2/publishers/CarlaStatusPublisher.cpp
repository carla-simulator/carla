// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaStatusPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

CarlaStatusPublisher::CarlaStatusPublisher()
  : PublisherBaseSensor(carla::ros2::types::ActorNameDefinition::CreateFromRoleName("status")),
    _impl(std::make_shared<CarlaStatusPublisherImpl>()) {}

bool CarlaStatusPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, get_topic_name(), get_topic_qos());
}

bool CarlaStatusPublisher::Publish() {
  return _impl->Publish();
}

bool CarlaStatusPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void CarlaStatusPublisher::UpdateCarlaStatus(const carla_msgs::msg::CarlaStatus& status) {
  _impl->Message() = status;
  _impl->SetMessageUpdated();
}
}  // namespace ros2
}  // namespace carla
