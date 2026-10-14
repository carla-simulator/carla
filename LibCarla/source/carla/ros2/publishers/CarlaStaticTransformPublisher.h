// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>

#include "carla/geom/Transform.h"
#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"

#include "carla/ros2/types/msg/TFMessage.h"

namespace carla {
namespace ros2 {

  /// Publishes static transforms as a latched tf2_msgs/TFMessage on
  /// rt/tf_static. The transient_local durability lets late-joining
  /// subscribers receive the static frames without CARLA re-publishing.
  class CarlaStaticTransformPublisher : public BasePublisher {
    public:
      struct StaticTransformMsgTraits {
        using msg_type = msg::TFMessage;
      };

      CarlaStaticTransformPublisher() :
        BasePublisher("rt/tf_static"),
        _impl(std::make_shared<PublisherImpl<StaticTransformMsgTraits>>()) {
          PublisherQos qos;
          qos.durability = DurabilityKind::TransientLocal;
          if (!_impl->Init(GetBaseTopicName(), qos)) {
            log_warning("CarlaStaticTransformPublisher: Init failed for topic: ", GetBaseTopicName());
          }
      }

      bool Publish() {
        return _impl->Publish();
      }

      /// @param transform parent-to-child transform in UE coordinates
      ///        (meters, degrees)
      bool Write(
          int32_t seconds,
          uint32_t nanoseconds,
          std::string frame_id,
          std::string child_frame_id,
          const geom::Transform &transform);

    private:
      std::shared_ptr<PublisherImpl<StaticTransformMsgTraits>> _impl;
  };

}  // namespace ros2
}  // namespace carla
