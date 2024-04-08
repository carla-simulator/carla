// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/VehicleControlSubscriber.h"

#include "carla/ros2/impl/fastdds/DdsSubscriberImpl.h"

namespace carla {
namespace ros2 {

VehicleControlSubscriber::VehicleControlSubscriber(ROS2NameRecord& parent,
                                                   carla::ros2::types::VehicleControlCallback vehicle_control_callback)
  : SubscriberBase(parent),
    _impl(std::make_shared<VehicleControlSubscriberImpl>()),
    _vehicle_control_callback(vehicle_control_callback) {}

bool VehicleControlSubscriber::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, _parent.get_topic_name("vehicle_control_cmd"), get_topic_qos());
}

const carla_msgs::msg::CarlaVehicleControl& VehicleControlSubscriber::GetMessage() {
  return _impl->GetMessage();
};

bool VehicleControlSubscriber::IsAlive() const {
  return _impl->IsAlive();
}

bool VehicleControlSubscriber::HasNewMessage() const {
  return _impl->HasNewMessage();
}

void VehicleControlSubscriber::ProcessMessages() {
  while (IsAlive() && HasNewMessage()) {
    _vehicle_control_callback(carla::ros2::types::VehicleControl(GetMessage()));
  }
}

}  // namespace ros2
}  // namespace carla
