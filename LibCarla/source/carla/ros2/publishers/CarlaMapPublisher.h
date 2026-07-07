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
  ///
  /// std_msgs/String carries no Header, and that is deliberate: it keeps the
  /// topic wire-compatible with carla-ros-bridge's /carla/map, which existing
  /// tooling already consumes. The message therefore has no stamp or episode
  /// id of its own; its identity contract is positional instead. The writer
  /// cache holds exactly one sample (keep-last depth 1) and the map is
  /// re-published on every episode start / map load, overwriting that sample,
  /// so the latched payload a late joiner receives always describes the map
  /// of the current episode. Consumers that need to detect a map change
  /// should watch for a new sample arriving on this topic (the OpenDRIVE
  /// header's own revision/name attributes identify the map) rather than
  /// expect a ROS stamp.
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
