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
    _impl(std::make_shared<CarlaSynchronizationWindowSubscriberImpl>(*this)),
    _carla_server(carla_server) {}

bool CarlaSynchronizationWindowSubscriber::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, get_topic_name("control/synchronization_window"), get_topic_qos().reliable());
}

CarlaSynchronizationWindowSubscriber::~CarlaSynchronizationWindowSubscriber() {
  for (auto [client, participant] : _carla_synchronization_window_participants) {
    _carla_server.call_deregister_synchronization_participant(client, participant);
  }
}

void CarlaSynchronizationWindowSubscriber::PublisherConnected(
    carla::rpc::synchronization_client_id_type const &publisher) {
  auto carla_synchronization_window_participant = _carla_server.call_register_synchronization_participant(publisher);
  _carla_synchronization_window_participants.insert({publisher, carla_synchronization_window_participant});
  carla::log_debug("CarlaSynchronizationWindowSubscriber[", publisher, "]::PublisherConnected(",
                   _carla_synchronization_window_participants[publisher], ")");
}

void CarlaSynchronizationWindowSubscriber::PublisherDisconnected(
    carla::rpc::synchronization_client_id_type const &publisher) {
  carla::log_debug("CarlaSynchronizationWindowSubscriber[", publisher, "]::PublisherDisconnected(",
                   _carla_synchronization_window_participants[publisher], ")");
  _carla_server.call_deregister_synchronization_participant(publisher,
                                                            _carla_synchronization_window_participants[publisher]);
  _carla_synchronization_window_participants.erase(publisher);
}

void CarlaSynchronizationWindowSubscriber::ProcessMessages() {
  while (_impl->HasPublishersConnected() && _impl->HasNewMessage()) {
    auto const carla_synchronization_window_msg_entry = _impl->GetMessageEntry();
    _carla_synchronization_target_game_time =
        carla_synchronization_window_msg_entry.message.synchronization_window_target_game_time();
    carla::log_debug("CarlaSynchronizationWindowSubscriber[", carla_synchronization_window_msg_entry.publisher,
                     "]::ProcessMessages(",
                     _carla_synchronization_window_participants[carla_synchronization_window_msg_entry.publisher],
                     ")=", _carla_synchronization_target_game_time);
    _carla_server.call_update_synchronization_window(
        carla_synchronization_window_msg_entry.publisher,
        _carla_synchronization_window_participants[carla_synchronization_window_msg_entry.publisher],
        _carla_synchronization_target_game_time);
  }
}

}  // namespace ros2
}  // namespace carla
