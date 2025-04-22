// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "BaseSubscriber.h"
#include "SubscriberImpl.h"

#include "carla/ros2/types/AckermannDriveStamped.h"
#include "carla/ros2/types/AckermannDriveStampedPubSubTypes.h"


#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

  class AckermannControlSubscriber : public BaseSubscriber {
    public:
      typedef ackermann_msgs::msg::AckermannDriveStamped msg_type;
      typedef ackermann_msgs::msg::AckermannDriveStampedPubSubType msg_pubsub_type;

      AckermannControlSubscriber(void* vehicle, const char* ros_name = "", const char* parent = "") :
        BaseSubscriber(vehicle, ros_name, parent),
        _impl(std::make_shared<SubscriberImpl<AckermannControlSubscriber>>()) {
          _impl->Init(this->GetTopicName());
        }

      std::string GetSubscriberType() override {
        return "/ackermann_control_cmd";
      }

      ROS2CallbackData GetMessage();
      void ProcessMessages(ActorCallback callback);

    private:
      std::shared_ptr<SubscriberImpl<AckermannControlSubscriber>> _impl;
  };

}  // namespace ros2
}  // namespace carla
