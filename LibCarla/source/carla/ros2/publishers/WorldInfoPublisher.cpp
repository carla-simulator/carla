// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "WorldInfoPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"
#include "carla/Version.h"

namespace carla {
namespace ros2 {

WorldInfoPublisher::WorldInfoPublisher(carla::rpc::RpcServerInterface &carla_server)
  : PublisherBase(carla::ros2::types::ActorNameDefinition::CreateFromRoleName("world_info")),
    _impl(std::make_shared<WorldInfoPublisherImpl>()),
    _carla_server(carla_server) {}

bool WorldInfoPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name(), get_topic_qos().keep_last(1));
}

bool WorldInfoPublisher::Publish() {
  return _impl->Publish();
}

bool WorldInfoPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

bool WorldInfoPublisher::ProcessMessages() {
  if ( _map_updated )
  {
    _impl->Message().carla_version(carla::version());
    _impl->Message().map_name(_carla_server.call_get_map_info().Get().name);
    _impl->Message().opendrive(_carla_server.call_get_map_data().Get());
    _impl->SetMessageUpdated();
    _map_updated = false;
  }
  return true;
}
}  // namespace ros2
}  // namespace carla
