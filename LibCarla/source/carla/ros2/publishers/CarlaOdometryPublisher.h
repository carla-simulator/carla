// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/BasePublisher.h"

#include <cstdint>
#include <memory>
#include <string>

namespace carla {
namespace ros2 {

template <typename Traits> class PublisherImpl;
struct CarlaOdometryMsgTraits;

// Publishes nav_msgs::msg::Odometry on <base>/odometry. Plain scalars only: the
// carla-ros2-native ExternalProject has no MsgPack/Boost for carla::geom.
class CarlaOdometryPublisher : public BasePublisher {
public:
  CarlaOdometryPublisher(std::string base_topic_name, std::string frame_id);
  ~CarlaOdometryPublisher() override;

  CarlaOdometryPublisher(const CarlaOdometryPublisher &) = delete;
  CarlaOdometryPublisher &operator=(const CarlaOdometryPublisher &) = delete;

  bool Publish() override;

  // Inputs are in CARLA's left-handed frame: world location (m), rotation
  // (degrees), body-frame linear velocity (m/s) and angular velocity (rad/s).
  bool Write(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      float x, float y, float z,
      float pitch_deg, float yaw_deg, float roll_deg,
      float vx, float vy, float vz,
      float wx, float wy, float wz);

private:
  std::shared_ptr<PublisherImpl<CarlaOdometryMsgTraits>> _impl;
};

}  // namespace ros2
}  // namespace carla
