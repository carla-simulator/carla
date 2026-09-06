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
struct CarlaGnssMsgTraits;

// Publishes sensor_msgs::msg::NavSatFix. Callers pass raw lat/lon/alt as
// doubles so the publisher header stays clear of carla/geom/*, which pulls
// MsgPack + Boost via carla/MsgPack.h — neither is on the include path of
// the carla-ros2-native ExternalProject that compiles these sources.
class CarlaGNSSPublisher : public BasePublisher {
public:
  CarlaGNSSPublisher(std::string base_topic_name, std::string frame_id);
  ~CarlaGNSSPublisher() override;

  CarlaGNSSPublisher(const CarlaGNSSPublisher &) = delete;
  CarlaGNSSPublisher &operator=(const CarlaGNSSPublisher &) = delete;
  CarlaGNSSPublisher(CarlaGNSSPublisher &&) noexcept = default;
  CarlaGNSSPublisher &operator=(CarlaGNSSPublisher &&) noexcept = default;

  bool Publish() override;
  bool Write(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      double latitude,
      double longitude,
      double altitude);

private:
  std::shared_ptr<PublisherImpl<CarlaGnssMsgTraits>> _impl;
};

}  // namespace ros2
}  // namespace carla
