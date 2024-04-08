// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/CarlaControlSubscriber.h"

#include "carla/ros2/impl/fastdds/DdsSubscriberImpl.h"

namespace carla {
namespace ros2 {

CarlaControlSubscriber::CarlaControlSubscriber(ROS2NameRecord &parent, carla::rpc::RpcServerInterface &carla_server)
  : SubscriberBase(parent), _impl(std::make_shared<CarlaControlSubscriberImpl>()), _carla_server(carla_server) {}

bool CarlaControlSubscriber::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  _carla_control_synchronization_participant =
      _carla_server.call_register_synchronization_participant(ThisAsSynchronizationClient());
  return _impl->Init(domain_participant, _parent.get_topic_name("control"), get_topic_qos().reliable());
}

CarlaControlSubscriber::~CarlaControlSubscriber() {
  _carla_server.call_deregister_synchronization_participant(ThisAsSynchronizationClient(),
                                                            _carla_control_synchronization_participant);
}

const carla_msgs::msg::CarlaControl &CarlaControlSubscriber::GetMessage() {
  return _impl->GetMessage();
};

bool CarlaControlSubscriber::IsAlive() const {
  return _impl->IsAlive();
}

bool CarlaControlSubscriber::HasNewMessage() const {
  return _impl->HasNewMessage();
}

void CarlaControlSubscriber::ProcessMessages() {
  while (IsAlive() && HasNewMessage()) {
    auto const carla_control_msg = GetMessage();
    auto const command = carla_control_msg.command();
    log_error("ROS2::ProcessMessages command =", std::to_string(command));
    switch (command) {
      case carla_msgs::msg::CarlaControl_Constants::PLAY:
        _carla_server.call_update_synchronization_window(ThisAsSynchronizationClient(),
                                                         _carla_control_synchronization_participant);
        break;
      case carla_msgs::msg::CarlaControl_Constants::PAUSE:
      case carla_msgs::msg::CarlaControl_Constants::STEP_ONCE:
        _carla_server.call_tick(ThisAsSynchronizationClient(), _carla_control_synchronization_participant);
        break;
    }
  }
}

}  // namespace ros2
}  // namespace carla
