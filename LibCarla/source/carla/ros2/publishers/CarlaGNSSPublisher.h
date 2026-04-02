// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/geom/GeoLocation.h"

#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"

#include "carla/ros2/types/NavSatFix.h"
#include "carla/ros2/types/NavSatFixPubSubTypes.h"

namespace carla {
namespace ros2 {

  class CarlaGNSSPublisher : public BasePublisher {
    public:
      struct GnssMsgTraits {
        using msg_type = sensor_msgs::msg::NavSatFix;
        using msg_pubsub_type = sensor_msgs::msg::NavSatFixPubSubType;
      };

      CarlaGNSSPublisher(std::string base_topic_name, std::string frame_id):
        BasePublisher(base_topic_name, frame_id),
        _impl(std::make_shared<PublisherImpl<GnssMsgTraits>>()) {
          _impl->Init(this->GetBaseTopicName());
      }

      bool Publish() {
        return _impl->Publish();
      }

      bool Write(int32_t seconds, uint32_t nanoseconds, const geom::GeoLocation data);

    private:
      std::shared_ptr<PublisherImpl<GnssMsgTraits>> _impl;
  };

}  // namespace ros2
}  // namespace carla
