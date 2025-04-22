// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "BaseSubscriber.h"
#include "SubscriberImpl.h"

#include "carla/ros2/types/CarlaEgoVehicleControl.h"
#include "carla/ros2/types/CarlaEgoVehicleControlPubSubTypes.h"

#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

  class CarlaEgoVehicleControlSubscriber : public BaseSubscriber {
    public:
      typedef carla_msgs::msg::CarlaEgoVehicleControl msg_type;
      typedef carla_msgs::msg::CarlaEgoVehicleControlPubSubType msg_pubsub_type;
 
      CarlaEgoVehicleControlSubscriber(void* vehicle, const char* ros_name = "", const char* parent = "") :
        BaseSubscriber(vehicle, ros_name, parent),
        _impl(std::make_shared<SubscriberImpl<CarlaEgoVehicleControlSubscriber>>()) {
          _impl->Init(this->GetTopicName());
        }

      std::string GetSubscriberType() override {
        return "/vehicle_control_cmd";
      }

      ROS2CallbackData GetMessage();
      void ProcessMessages(ActorCallback callback);

    private:
      std::shared_ptr<SubscriberImpl<CarlaEgoVehicleControlSubscriber>> _impl;
  };

}  // namespace ros2
}  // namespace carla
