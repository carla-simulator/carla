// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "BasePublisher.h"
#include "PublisherImpl.h"

#include "carla/ros2/types/Odometry.h"
#include "carla/ros2/types/OdometryPubSubTypes.h"

#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

  class OdometryPublisher : public BasePublisher {
    public:
      typedef nav_msgs::msg::Odometry msg_type;
      typedef nav_msgs::msg::OdometryPubSubType msg_pubsub_type;

      OdometryPublisher(void* vehicle, const char* ros_name = "", const char* parent = "") :
        BasePublisher(vehicle, ros_name, parent),
        _impl(std::make_shared<PublisherImpl<OdometryPublisher>>()) {
          _impl->Init(this->GetTopicName());
        }

      std::string GetPublisherType() override {
        return "/odometry";
      }

      bool IsPseudoPublisher() { return true; }

      bool Publish() {
        return _impl->Publish();
      }

      bool UpdateData(int32_t seconds, uint32_t nanoseconds, ActorCallback callback);

    private:
      std::shared_ptr<PublisherImpl<OdometryPublisher>> _impl;
  };

}  // namespace ros2
}  // namespace carla
