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
struct CarlaCollisionMsgTraits;

// Publishes carla_msgs::msg::CarlaCollisionEvent. Each event carries the
// other actor's id and the contact normal impulse (Newton-seconds). The
// impulse is passed as a float triple so the publisher header does not
// include carla::geom::Vector3D (which pulls MsgPack + Boost — neither is
// on the include path of the carla-ros2-native ExternalProject).
class CarlaCollisionPublisher : public BasePublisher {
public:
  CarlaCollisionPublisher(std::string base_topic_name, std::string frame_id);
  ~CarlaCollisionPublisher() override;

  CarlaCollisionPublisher(const CarlaCollisionPublisher &) = delete;
  CarlaCollisionPublisher &operator=(const CarlaCollisionPublisher &) = delete;
  CarlaCollisionPublisher(CarlaCollisionPublisher &&) noexcept = default;
  CarlaCollisionPublisher &operator=(CarlaCollisionPublisher &&) noexcept = default;

  bool Publish() override;
  bool Write(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      std::uint32_t other_actor_id,
      float impulse_x,
      float impulse_y,
      float impulse_z);

private:
  std::shared_ptr<PublisherImpl<CarlaCollisionMsgTraits>> _impl;
};

}  // namespace ros2
}  // namespace carla
