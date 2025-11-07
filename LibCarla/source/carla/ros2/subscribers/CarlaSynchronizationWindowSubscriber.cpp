// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/CarlaSynchronizationWindowSubscriber.h"

#include "carla/ros2/impl/DdsSubscriberImpl.h"

namespace carla {
namespace ros2 {

CarlaSynchronizationWindowSubscriber::CarlaSynchronizationWindowSubscriber(ROS2NameRecord &parent,
                                                                           carla::rpc::RpcServerInterface &carla_server)
  : SubscriberBaseSynchronizationClient(parent, carla_server),
    _impl(std::make_shared<CarlaSynchronizationWindowSubscriberImpl>(*this))
  {}

bool CarlaSynchronizationWindowSubscriber::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, get_topic_name("control/synchronization_window"), get_topic_qos().reliable());
}

CarlaSynchronizationWindowSubscriber::~CarlaSynchronizationWindowSubscriber() {
}

void CarlaSynchronizationWindowSubscriber::ProcessMessages() {
  while (_impl->HasPublishersConnected() && _impl->HasNewMessage()) {
    auto const carla_synchronization_window_msg_entry = _impl->GetMessageEntry();
    auto carla_synchronization_target_game_time =
        carla_synchronization_window_msg_entry.message.synchronization_window_target_game_time();
    auto const synchronization_participant = GetSynchronizationParticipant(carla_synchronization_window_msg_entry.publisher);

    carla::log_debug("CarlaSynchronizationWindowSubscriber[", ThisAsSynchronizationClient(),
                     "]::ProcessMessages(", carla_synchronization_window_msg_entry.publisher, ", ",
                     synchronization_participant,
                     ")=", carla_synchronization_target_game_time);
    _carla_server.call_update_synchronization_window(
        ThisAsSynchronizationClient(),
        synchronization_participant,
        carla_synchronization_target_game_time);
  }
}

}  // namespace ros2
}  // namespace carla
