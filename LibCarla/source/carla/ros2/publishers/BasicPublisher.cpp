// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/BasicPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/types/String.h"
#include "carla/ros2/types/StringPubSubTypes.h"

namespace carla {
namespace ros2 {

struct BasicPublisherMsgTraits {
  using msg_type = std_msgs::msg::String;
  using msg_pubsub_type = std_msgs::msg::StringPubSubType;
};

BasicPublisher::BasicPublisher(std::string ros_name, std::string parent)
  : BasePublisher(std::move(parent), std::move(ros_name)),
    _impl(std::make_shared<PublisherImpl<BasicPublisherMsgTraits>>()) {}

BasicPublisher::~BasicPublisher() = default;

bool BasicPublisher::Init() {
  return _impl->Init("rt/basic_publisher_example");
}

bool BasicPublisher::Publish() {
  return _impl->Publish();
}

void BasicPublisher::SetData(const std::string &msg) {
  _impl->GetMessage()->data(msg);
}

}  // namespace ros2
}  // namespace carla
