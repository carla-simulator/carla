// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/TrafficLightData.h"
#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"

#include "carla/ros2/types/msg/CarlaTrafficLightStatusList.h"

namespace carla {
namespace ros2 {

  /// Publishes the state of every traffic light in the map as a latched
  /// carla_msgs/CarlaTrafficLightStatusList on rt/carla/traffic_lights/status.
  /// The full list is rewritten only when any light changed state since the
  /// last publish; the transient_local durability keeps the latest list
  /// available to late-joining subscribers between changes.
  class CarlaTrafficLightStatusPublisher : public BasePublisher {
    public:
      struct StatusListMsgTraits {
        using msg_type = msg::CarlaTrafficLightStatusList;
      };

      CarlaTrafficLightStatusPublisher() :
        BasePublisher("rt/carla/traffic_lights/status"),
        _impl(std::make_shared<PublisherImpl<StatusListMsgTraits>>()) {
          PublisherQos qos;
          qos.durability = DurabilityKind::TransientLocal;
          if (!_impl->Init(GetBaseTopicName(), qos)) {
            log_warning("CarlaTrafficLightStatusPublisher: Init failed for topic: ", GetBaseTopicName());
          }
      }

      bool Publish() {
        return _impl->Publish();
      }

      /// Rewrites the message when the states differ from the last written
      /// list and returns true; returns false when nothing changed so the
      /// caller can skip the publish.
      bool Write(const std::vector<TrafficLightState> &states);

      /// True when both lists describe the same lights in the same order
      /// with the same states.
      static bool StatesEqual(
          const std::vector<TrafficLightState> &lhs,
          const std::vector<TrafficLightState> &rhs) {
        if (lhs.size() != rhs.size()) {
          return false;
        }
        for (size_t i = 0u; i < lhs.size(); ++i) {
          if (lhs[i].id != rhs[i].id || lhs[i].state != rhs[i].state) {
            return false;
          }
        }
        return true;
      }

    private:
      std::shared_ptr<PublisherImpl<StatusListMsgTraits>> _impl;
      std::vector<TrafficLightState> _last_states;
      bool _has_last_states{false};
  };

}  // namespace ros2
}  // namespace carla
