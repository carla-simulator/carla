// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
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
      struct ControlMsgTraits {
        using msg_type = carla_msgs::msg::CarlaEgoVehicleControl;
        using msg_pubsub_type = carla_msgs::msg::CarlaEgoVehicleControlPubSubType;
      };


      CarlaEgoVehicleControlSubscriber(void* vehicle, std::string base_topic_name, std::string frame_id) :
        BaseSubscriber(vehicle, base_topic_name, frame_id),
        _impl(std::make_shared<SubscriberImpl<ControlMsgTraits>>()) {
          _impl->Init(this->GetBaseTopicName() + "/vehicle_control_cmd");
        }

      ROS2CallbackData GetMessage();
      void ProcessMessages(ActorCallback callback);

    private:
      std::shared_ptr<SubscriberImpl<ControlMsgTraits>> _impl;
  };

}  // namespace ros2
}  // namespace carla
