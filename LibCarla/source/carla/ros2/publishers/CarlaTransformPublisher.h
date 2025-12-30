// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/geom/Transform.h"

#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"

#include "carla/ros2/types/TFMessage.h"
#include "carla/ros2/types/TFMessagePubSubTypes.h"

namespace carla {
namespace ros2 {

  class CarlaTransformPublisher : public BasePublisher {
    public:
      struct TransformMsgTraits {
        using msg_type = tf2_msgs::msg::TFMessage;
        using msg_pubsub_type = tf2_msgs::msg::TFMessagePubSubType;
      };

      CarlaTransformPublisher() :
        BasePublisher("rt/tf"),
        _impl(std::make_shared<PublisherImpl<TransformMsgTraits>>()) {
          _impl->Init(GetBaseTopicName());
      }

      bool Publish() {
        return _impl->Publish();
      }

      bool Write(int32_t seconds, uint32_t nanoseconds, std::string frame_id, std::string child_frame_id, geom::Transform transform);

    private:
      geometry_msgs::msg::Transform ComputeTransform(std::string frame_id, geom::Transform current_transform);

    private:
      std::shared_ptr<PublisherImpl<TransformMsgTraits>> _impl;

      std::unordered_map<std::string, std::pair<geom::Transform, geometry_msgs::msg::Transform>> _last_transforms;
  };

}  // namespace ros2
}  // namespace carla
