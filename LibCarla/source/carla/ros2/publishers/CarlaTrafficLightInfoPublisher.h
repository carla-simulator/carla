// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/TrafficLightData.h"
#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"

#include "carla/ros2/types/msg/CarlaTrafficLightInfoList.h"

namespace carla {
namespace ros2 {

  /// Publishes the static description of every traffic light in the map as a
  /// latched carla_msgs/CarlaTrafficLightInfoList on
  /// rt/carla/traffic_lights/info. Published once per episode; the
  /// transient_local durability lets late-joining subscribers receive the
  /// list without CARLA re-publishing it.
  class CarlaTrafficLightInfoPublisher : public BasePublisher {
    public:
      struct InfoListMsgTraits {
        using msg_type = msg::CarlaTrafficLightInfoList;
      };

      CarlaTrafficLightInfoPublisher() :
        BasePublisher("rt/carla/traffic_lights/info"),
        _impl(std::make_shared<PublisherImpl<InfoListMsgTraits>>()) {
          PublisherQos qos;
          qos.durability = DurabilityKind::TransientLocal;
          if (!_impl->Init(GetBaseTopicName(), qos)) {
            log_warning("CarlaTrafficLightInfoPublisher: Init failed for topic: ", GetBaseTopicName());
          }
      }

      bool Publish() {
        return _impl->Publish();
      }

      bool Write(const std::vector<TrafficLightInfo> &info);

    private:
      std::shared_ptr<PublisherImpl<InfoListMsgTraits>> _impl;
  };

}  // namespace ros2
}  // namespace carla
