// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaMapPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/middleware/QosProfile.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/types/msg/String.h"

namespace carla {
namespace ros2 {

struct CarlaMapMsgTraits {
  using msg_type = msg::String;
};

CarlaMapPublisher::CarlaMapPublisher()
    : BasePublisher("rt/carla/map"),
      _impl(std::make_shared<PublisherImpl<CarlaMapMsgTraits>>()) {
  auto qos = QosProfile::ReliableTransientLocal();
  if (!_impl->Init(GetBaseTopicName(), qos)) {
    log_warning("CarlaMapPublisher: failed to initialise writer for ",
                GetBaseTopicName());
  }
}

CarlaMapPublisher::~CarlaMapPublisher() = default;

bool CarlaMapPublisher::Publish() {
  return _impl->Publish();
}

bool CarlaMapPublisher::Write(const std::string &open_drive) {
  _impl->GetMessage()->data = open_drive;
  return true;
}

}  // namespace ros2
}  // namespace carla
