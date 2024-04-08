// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapPublisher.h"

#include "carla/ros2/impl/fastdds/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

MapPublisher::MapPublisher()
  : PublisherBase(carla::ros2::types::ActorNameDefinition::CreateFromRoleName("map")),
    _impl(std::make_shared<MapPublisherImpl>()) {}

bool MapPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name(), get_topic_qos());
}

bool MapPublisher::Publish() {
  return _impl->Publish();
}

bool MapPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void MapPublisher::UpdateData(std::string const &data) {
  _impl->Message().data(data);
  _impl->SetMessageUpdated();
}
}  // namespace ros2
}  // namespace carla
