// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaActorListPublisher.h"

#include "carla/ros2/impl/fastdds/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

CarlaActorListPublisher::CarlaActorListPublisher()
  : PublisherBase(carla::ros2::types::ActorNameDefinition::CreateFromRoleName("actor_list")),
    _impl(std::make_shared<CarlaActorListPublisherImpl>()) {}

bool CarlaActorListPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, get_topic_name(), get_topic_qos());
}

bool CarlaActorListPublisher::Publish() {
  return _impl->Publish();
}

bool CarlaActorListPublisher::SubsribersConnected() const {
  return _impl->SubsribersConnected();
}

void CarlaActorListPublisher::UpdateCarlaActorList(const carla_msgs::msg::CarlaActorList& status) {
  _impl->Message() = status;
  _impl->SetMessageUpdated();
}
}  // namespace ros2
}  // namespace carla
