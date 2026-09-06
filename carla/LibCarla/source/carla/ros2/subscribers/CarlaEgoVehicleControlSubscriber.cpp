// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/CarlaEgoVehicleControlSubscriber.h"

#include "carla/Logging.h"
#include "carla/ros2/ROS2CallbackData.h"
#include "carla/ros2/subscribers/SubscriberImpl.h"
#include "carla/ros2/types/CarlaEgoVehicleControl.h"
#include "carla/ros2/types/CarlaEgoVehicleControlPubSubTypes.h"

namespace carla {
namespace ros2 {

struct CarlaEgoVehicleControlTraits {
  using msg_type = carla_msgs::msg::CarlaEgoVehicleControl;
  using msg_pubsub_type = carla_msgs::msg::CarlaEgoVehicleControlPubSubType;
};

CarlaEgoVehicleControlSubscriber::CarlaEgoVehicleControlSubscriber(
    void *vehicle, std::string base_topic_name, std::string frame_id)
  : BaseSubscriber(vehicle, std::move(base_topic_name), std::move(frame_id)),
    _impl(std::make_shared<SubscriberImpl<CarlaEgoVehicleControlTraits>>()) {
  if (!_impl->Init(this->GetBaseTopicName() + "/vehicle_control_cmd")) {
    log_error("CarlaEgoVehicleControlSubscriber failed to initialize on base topic",
              this->GetBaseTopicName());
  }
}

CarlaEgoVehicleControlSubscriber::~CarlaEgoVehicleControlSubscriber() = default;

ROS2CallbackData CarlaEgoVehicleControlSubscriber::GetMessage() {
  auto message = _impl->GetMessage();

  VehicleControl control;
  control.throttle = message.throttle();
  control.steer = message.steer();
  control.brake = message.brake();
  control.hand_brake = message.hand_brake();
  control.reverse = message.reverse();
  control.gear = message.gear();
  control.manual_gear_shift = message.manual_gear_shift();
  return control;
}

void CarlaEgoVehicleControlSubscriber::ProcessMessages(ActorCallback callback) {
  if (_impl->HasNewMessage()) {
    auto control = this->GetMessage();
    callback(this->GetActor(), control);
  }
}

}  // namespace ros2
}  // namespace carla
