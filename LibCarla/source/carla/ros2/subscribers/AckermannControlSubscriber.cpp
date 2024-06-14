// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/AckermannControlSubscriber.h"

#include "carla/ros2/impl/DdsSubscriberImpl.h"

namespace carla {
namespace ros2 {

AckermannControlSubscriber::AckermannControlSubscriber(
    ROS2NameRecord& parent, carla::ros2::types::VehicleAckermannControlCallback vehicle_ackermann_control_callback)
  : SubscriberBase(parent),
    _impl(std::make_shared<AckermannControlSubscriberImpl>(*this)),
    _vehicle_ackermann_control_callback(vehicle_ackermann_control_callback) {}

bool AckermannControlSubscriber::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, get_topic_name("control/vehicle_ackermann_drive_cmd"), get_topic_qos());
}

void AckermannControlSubscriber::ProcessMessages() {
  while (_impl->HasPublishersConnected() && _impl->HasNewMessage()) {
    _vehicle_ackermann_control_callback(carla::ros2::types::VehicleAckermannControl(_impl->GetMessage()));
  }
}

}  // namespace ros2
}  // namespace carla
