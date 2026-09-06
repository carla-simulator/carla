// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/BasePublisher.h"

#include <cstdint>
#include <memory>

namespace carla {
namespace ros2 {

template <typename Traits> class PublisherImpl;
struct CarlaClockMsgTraits;

// Global ROS 2 clock publisher: writes rosgraph_msgs::msg::Clock to the
// well-known "rt/clock" topic. Single-instance — ROS2::Enable() constructs
// it once and SetTimestamp() drives every tick.
class CarlaClockPublisher : public BasePublisher {
public:
  CarlaClockPublisher();
  ~CarlaClockPublisher() override;

  CarlaClockPublisher(const CarlaClockPublisher &) = delete;
  CarlaClockPublisher &operator=(const CarlaClockPublisher &) = delete;
  CarlaClockPublisher(CarlaClockPublisher &&) noexcept = default;
  CarlaClockPublisher &operator=(CarlaClockPublisher &&) noexcept = default;

  bool Publish() override;
  bool Write(std::int32_t seconds, std::uint32_t nanoseconds);

private:
  std::shared_ptr<PublisherImpl<CarlaClockMsgTraits>> _impl;
};

}  // namespace ros2
}  // namespace carla
