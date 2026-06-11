// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>

#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"

#include "carla/ros2/types/msg/String.h"

namespace carla {
namespace ros2 {

  /// Publishes the OpenDRIVE description of the current map as a latched
  /// std_msgs/String on rt/carla/map. The transient_local durability lets
  /// late-joining subscribers receive the map without CARLA re-publishing it.
  class CarlaMapPublisher : public BasePublisher {
    public:
      struct MapMsgTraits {
        using msg_type = msg::String;
      };

      CarlaMapPublisher() :
        BasePublisher("rt/carla/map"),
        _impl(std::make_shared<PublisherImpl<MapMsgTraits>>()) {
          PublisherQos qos;
          qos.durability = DurabilityKind::TransientLocal;
          if (!_impl->Init(GetBaseTopicName(), qos)) {
            log_warning("CarlaMapPublisher: Init failed for topic: ", GetBaseTopicName());
          }
      }

      bool Publish() {
        return _impl->Publish();
      }

      bool Write(const std::string& open_drive);

    private:
      std::shared_ptr<PublisherImpl<MapMsgTraits>> _impl;
  };

}  // namespace ros2
}  // namespace carla
