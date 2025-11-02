// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/geom/Vector3D.h"

#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"

#include "carla/ros2/types/CarlaCollisionEvent.h"
#include "carla/ros2/types/CarlaCollisionEventPubSubTypes.h"

namespace carla {
namespace ros2 {

  class CarlaCollisionPublisher : public BasePublisher {
    public:
      struct CollisionMsgTraits {
        using msg_type = carla_msgs::msg::CarlaCollisionEvent;
        using msg_pubsub_type = carla_msgs::msg::CarlaCollisionEventPubSubType;
      };

      CarlaCollisionPublisher(std::string base_topic_name, std::string frame_id) :
        BasePublisher(base_topic_name, frame_id),
        _impl(std::make_shared<PublisherImpl<CollisionMsgTraits>>()) {
          _impl->Init(this->GetBaseTopicName());
      }

      bool Publish() {
        return _impl->Publish();
      }

      bool Write(int32_t seconds, uint32_t nanoseconds, uint32_t actor_id, geom::Vector3D impulse);

    private:
      std::shared_ptr<PublisherImpl<CollisionMsgTraits>> _impl;
  };

}  // namespace ros2
}  // namespace carla

