// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/BasePublisher.h"

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace carla {
namespace ros2 {

template <typename Traits> class PublisherImpl;
struct CarlaTransformMsgTraits;

// Publishes tf2_msgs::msg::TFMessage to the global "rt/tf" topic. Each
// sensor owns its own instance and contributes one TransformStamped per
// tick; subscribers see the full set fanned out across the topic. Callers
// hand in raw float translation + Euler degrees so the header does not
// include carla::geom::Transform (pulls MsgPack + Boost via
// carla/MsgPack.h — neither is on the include path of the
// carla-ros2-native ExternalProject).
class CarlaTransformPublisher : public BasePublisher {
public:
  CarlaTransformPublisher();
  ~CarlaTransformPublisher() override;

  CarlaTransformPublisher(const CarlaTransformPublisher &) = delete;
  CarlaTransformPublisher &operator=(const CarlaTransformPublisher &) = delete;
  CarlaTransformPublisher(CarlaTransformPublisher &&) noexcept = default;
  CarlaTransformPublisher &operator=(CarlaTransformPublisher &&) noexcept = default;

  bool Publish() override;

  // Builds a TransformStamped(parent_frame_id -> child_frame_id) from the
  // given CARLA-handed translation + Euler-degree rotation and stages it
  // for the next Publish().
  bool Write(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      const std::string &parent_frame_id,
      const std::string &child_frame_id,
      float tx, float ty, float tz,
      float pitch_deg, float yaw_deg, float roll_deg);

  // Cache the last (input, output) pair per child_frame_id so a static
  // sensor doesn't pay the quaternion conversion on every tick. Public so
  // the equality helper in the .cpp can pattern-match.
  struct CachedTransform {
    float tx;
    float ty;
    float tz;
    float pitch;
    float yaw;
    float roll;
    std::array<float, 3> translation;
    std::array<float, 4> rotation;
  };

private:
  std::shared_ptr<PublisherImpl<CarlaTransformMsgTraits>> _impl;
  std::unordered_map<std::string, CachedTransform> _last_transforms;
};

}  // namespace ros2
}  // namespace carla
