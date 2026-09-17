// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>

#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"
#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"

#include "carla/ros2/types/msg/Odometry.h"

namespace carla {
namespace ros2 {

  /// Publishes the ground-truth pose and body-frame twist of a registered
  /// vehicle as nav_msgs/Odometry on <base_topic>/odometry, once per frame.
  class CarlaOdometryPublisher : public BasePublisher {
    public:
      struct OdometryMsgTraits {
        using msg_type = msg::Odometry;
      };

      CarlaOdometryPublisher(std::string base_topic_name) :
        BasePublisher(base_topic_name + "/odometry"),
        _impl(std::make_shared<PublisherImpl<OdometryMsgTraits>>()) {
          if (!_impl->Init(GetBaseTopicName())) {
            log_warning("CarlaOdometryPublisher: Init failed for topic: ", GetBaseTopicName());
          }
      }

      bool Publish() {
        return _impl->Publish();
      }

      /// @param transform vehicle transform in UE coordinates (meters, degrees)
      /// @param linear_velocity world-frame velocity in m/s, UE coordinates
      /// @param angular_velocity world-frame angular velocity in deg/s, UE coordinates
      bool Write(
          int32_t seconds,
          uint32_t nanoseconds,
          std::string frame_id,
          std::string child_frame_id,
          const geom::Transform &transform,
          const geom::Vector3D &linear_velocity,
          const geom::Vector3D &angular_velocity);

    private:
      std::shared_ptr<PublisherImpl<OdometryMsgTraits>> _impl;
  };

}  // namespace ros2
}  // namespace carla
