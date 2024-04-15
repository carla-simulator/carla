// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/CarlaSynchronizationWindowSubscriber.h"

#include "carla/ros2/impl/DdsSubscriberImpl.h"

namespace carla {
namespace ros2 {

CarlaSynchronizationWindowSubscriber::CarlaSynchronizationWindowSubscriber(ROS2NameRecord &parent,
                                                                           carla::rpc::RpcServerInterface &carla_server)
  : SubscriberBase(parent),
    _impl(std::make_shared<CarlaSynchronizationWindowSubscriberImpl>()),
    _carla_server(carla_server) {}

bool CarlaSynchronizationWindowSubscriber::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  _carla_synchronization_window_participant =
      _carla_server.call_register_synchronization_participant(ThisAsSynchronizationWindowClient());
  return _impl->Init(domain_participant, _parent.get_topic_name("synchronization_window"), get_topic_qos().reliable());
}

CarlaSynchronizationWindowSubscriber::~CarlaSynchronizationWindowSubscriber() {
  _carla_server.call_deregister_synchronization_participant(ThisAsSynchronizationWindowClient(),
                                                            _carla_synchronization_window_participant);
}

const carla_msgs::msg::CarlaSynchronizationWindow &CarlaSynchronizationWindowSubscriber::GetMessage() {
  return _impl->GetMessage();
};

bool CarlaSynchronizationWindowSubscriber::IsAlive() const {
  return _impl->IsAlive();
}

bool CarlaSynchronizationWindowSubscriber::HasNewMessage() const {
  return _impl->HasNewMessage();
}

void CarlaSynchronizationWindowSubscriber::ProcessMessages() {
  while (IsAlive() && HasNewMessage()) {
    auto const carla_synchronization_window_msg = GetMessage();
    _carla_synchronization_target_game_time =
        carla_synchronization_window_msg.synchronization_window_target_game_time();
    _carla_server.call_update_synchronization_window(ThisAsSynchronizationWindowClient(),
                                                     _carla_synchronization_window_participant,
                                                     _carla_synchronization_target_game_time);
  }
  if (!IsAlive() && (_carla_synchronization_target_game_time > carla::rpc::NO_SYNC_TARGET_GAME_TIME)) {
    _carla_synchronization_target_game_time = carla::rpc::NO_SYNC_TARGET_GAME_TIME;
    _carla_server.call_update_synchronization_window(ThisAsSynchronizationWindowClient(),
                                                     _carla_synchronization_window_participant,
                                                     _carla_synchronization_target_game_time);
  }
}

}  // namespace ros2
}  // namespace carla
