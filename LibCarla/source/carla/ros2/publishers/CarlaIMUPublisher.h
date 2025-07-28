// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/geom/Vector3D.h"

#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"

#include "carla/ros2/types/Imu.h"
#include "carla/ros2/types/ImuPubSubTypes.h"

namespace carla {
namespace ros2 {

  class CarlaIMUPublisher : public BasePublisher {
    public:
      struct ImuMsgTraits {
        using msg_type = sensor_msgs::msg::Imu;
        using msg_pubsub_type = sensor_msgs::msg::ImuPubSubType;
      };

      CarlaIMUPublisher(std::string base_topic_name, std::string frame_id) :
        BasePublisher(base_topic_name, frame_id),
        _impl(std::make_shared<PublisherImpl<ImuMsgTraits>>()) {
          _impl->Init(this->GetBaseTopicName());
      }

      bool Publish() {
        return _impl->Publish();
      }

      bool Write(int32_t seconds, uint32_t nanoseconds, geom::Vector3D accelerometer, geom::Vector3D gyroscope, float compass);

    private:
      std::shared_ptr<PublisherImpl<ImuMsgTraits>> _impl;
  };

}  // namespace ros2
}  // namespace carla
