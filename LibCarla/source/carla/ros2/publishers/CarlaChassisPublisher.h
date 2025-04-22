// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "BasePublisher.h"
#include "PublisherImpl.h"

#include "carla/ros2/types/CarlaChassisStatus.h"
#include "carla/ros2/types/CarlaChassisStatusPubSubTypes.h"

#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

  class CarlaChassisPublisher : public BasePublisher {
    public:
      typedef carla_msgs::msg::CarlaChassisStatus msg_type;
      typedef carla_msgs::msg::CarlaChassisStatusPubSubType msg_pubsub_type;

      CarlaChassisPublisher(void* vehicle, const char* ros_name = "", const char* parent = "") :
        BasePublisher(vehicle, ros_name, parent),
        _impl(std::make_shared<PublisherImpl<CarlaChassisPublisher>>()) {
          _impl->Init(this->GetTopicName());
        }

      std::string GetPublisherType() override {
        return "/chassis";
      }

      bool IsPseudoPublisher() { return true; }

      bool Publish() {
        return _impl->Publish();
      }

      bool UpdateData(int32_t seconds, uint32_t nanoseconds, ActorCallback callback);

    private:
      std::shared_ptr<PublisherImpl<CarlaChassisPublisher>> _impl;
  };

}  // namespace ros2
}  // namespace carla
