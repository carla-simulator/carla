// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/CarlaControlSubscriber.h"

#include "carla/ros2/impl/DdsSubscriberImpl.h"

namespace carla {
namespace ros2 {

CarlaControlSubscriber::CarlaControlSubscriber(ROS2NameRecord &parent, carla::rpc::RpcServerInterface &carla_server)
  : SubscriberBaseSynchronizationClient(parent, carla_server), _impl(std::make_shared<CarlaControlSubscriberImpl>(*this)) {}

bool CarlaControlSubscriber::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, get_topic_name("control/carla_control"), get_topic_qos().reliable());
}

CarlaControlSubscriber::~CarlaControlSubscriber() {
}

void CarlaControlSubscriber::ProcessMessages() {
  while (_impl->HasPublishersConnected() && _impl->HasNewMessage()) {

    auto const carla_control_msg_entry = _impl->GetMessageEntry();
    auto const command = carla_control_msg_entry.message.command();
    auto const synchronization_participant = GetSynchronizationParticipant(carla_control_msg_entry.publisher);
    carla::log_debug("CarlaControlSubscriber[", ThisAsSynchronizationClient(),
                     "]::ProcessMessages(", carla_control_msg_entry.publisher, ", ",
                     synchronization_participant,
                     ") command =", std::to_string(command));
    switch (command) {
      case carla_msgs::msg::CarlaControl_Constants::PLAY:
        _carla_server.call_update_synchronization_window(ThisAsSynchronizationClient(),
                                                         synchronization_participant);
        break;
      case carla_msgs::msg::CarlaControl_Constants::PAUSE:
      case carla_msgs::msg::CarlaControl_Constants::STEP_ONCE:
        _carla_server.call_tick(ThisAsSynchronizationClient(), synchronization_participant);
        break;
    }
  }
}

}  // namespace ros2
}  // namespace carla
