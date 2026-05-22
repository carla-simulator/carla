// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/BasePublisher.h"

#include <memory>
#include <string>

namespace carla {
namespace ros2 {

template <typename Traits> class PublisherImpl;
struct BasicPublisherMsgTraits;

// Developer scaffold: publishes a std_msgs::String to a fixed topic. Only
// reachable from the WITH_ROS2_DEMO code paths in ROS2.cpp, but compiles
// unconditionally so the carla-ros2-native ExternalProject can pick it up.
class BasicPublisher : public BasePublisher {
public:
  BasicPublisher(std::string ros_name, std::string parent);
  ~BasicPublisher() override;

  BasicPublisher(const BasicPublisher &) = delete;
  BasicPublisher &operator=(const BasicPublisher &) = delete;
  BasicPublisher(BasicPublisher &&) noexcept = default;
  BasicPublisher &operator=(BasicPublisher &&) noexcept = default;

  bool Init();
  bool Publish() override;
  void SetData(const std::string &msg);

private:
  std::shared_ptr<PublisherImpl<BasicPublisherMsgTraits>> _impl;
};

}  // namespace ros2
}  // namespace carla
