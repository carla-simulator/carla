// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaGNSSPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/types/NavSatFix.h"
#include "carla/ros2/types/NavSatFixPubSubTypes.h"

namespace carla {
namespace ros2 {

struct CarlaGnssMsgTraits {
  using msg_type = sensor_msgs::msg::NavSatFix;
  using msg_pubsub_type = sensor_msgs::msg::NavSatFixPubSubType;
};

CarlaGNSSPublisher::CarlaGNSSPublisher(
    std::string base_topic_name, std::string frame_id)
  : BasePublisher(std::move(base_topic_name), std::move(frame_id)),
    _impl(std::make_shared<PublisherImpl<CarlaGnssMsgTraits>>()) {
  if (!_impl->Init(GetBaseTopicName())) {
    log_error("CarlaGNSSPublisher: failed to initialise writer for", GetBaseTopicName());
  }
}

CarlaGNSSPublisher::~CarlaGNSSPublisher() = default;

bool CarlaGNSSPublisher::Publish() {
  return _impl->Publish();
}

bool CarlaGNSSPublisher::Write(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    double latitude,
    double longitude,
    double altitude) {
  auto *message = _impl->GetMessage();
  message->header().stamp().sec(seconds);
  message->header().stamp().nanosec(nanoseconds);
  message->header().frame_id(GetFrameId());

  message->latitude(latitude);
  message->longitude(longitude);
  message->altitude(altitude);
  return true;
}

}  // namespace ros2
}  // namespace carla
