// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "AckermannControlSubscriber.h"

#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

  ROS2CallbackData AckermannControlSubscriber::GetMessage() {
    auto message = _impl->GetMessage();

    AckermannControl control;
    control.steer = message.drive.steering_angle;
    control.steer_speed = message.drive.steering_angle_velocity;
    control.speed = message.drive.speed;
    control.acceleration = message.drive.acceleration;
    control.jerk = message.drive.jerk;
    return control;
  }

  void AckermannControlSubscriber::ProcessMessages(ActorCallback callback) {
    if (_impl->HasNewMessage()) {
      auto control = this->GetMessage();
      callback(this->GetActor(), control);
    }
  }

}  // namespace ros2
}  // namespace carla
