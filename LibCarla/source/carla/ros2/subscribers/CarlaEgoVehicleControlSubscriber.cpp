#include "CarlaEgoVehicleControlSubscriber.h"

#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

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
