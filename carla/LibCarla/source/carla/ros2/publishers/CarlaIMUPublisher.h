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
struct CarlaImuMsgTraits;

// Publishes sensor_msgs::msg::Imu. Callers pass accelerometer / gyroscope
// reading as float triples and a compass heading (radians, magnetic-north
// clockwise). The orientation quaternion comes from the header-only
// ImuMath.h seam (PR-1's `a424ff2ec` correction, pinned by
// test_imu_compass.cpp). Header stays clear of carla::geom::Vector3D
// because that pulls MsgPack + Boost — neither is on the include path of
// the carla-ros2-native ExternalProject that compiles these sources.
class CarlaIMUPublisher : public BasePublisher {
public:
  CarlaIMUPublisher(std::string base_topic_name, std::string frame_id);
  ~CarlaIMUPublisher() override;

  CarlaIMUPublisher(const CarlaIMUPublisher &) = delete;
  CarlaIMUPublisher &operator=(const CarlaIMUPublisher &) = delete;
  CarlaIMUPublisher(CarlaIMUPublisher &&) noexcept = default;
  CarlaIMUPublisher &operator=(CarlaIMUPublisher &&) noexcept = default;

  bool Publish() override;
  bool Write(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      float accel_x, float accel_y, float accel_z,
      float gyro_x, float gyro_y, float gyro_z,
      float compass);

private:
  std::shared_ptr<PublisherImpl<CarlaImuMsgTraits>> _impl;
};

}  // namespace ros2
}  // namespace carla
