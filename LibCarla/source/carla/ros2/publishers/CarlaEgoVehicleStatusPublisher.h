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
#include "carla/rpc/VehicleControl.h"

#include "carla/ros2/types/msg/CarlaEgoVehicleStatus.h"

namespace carla {
namespace ros2 {

  /// Publishes the current speed, acceleration, orientation and applied
  /// control of a registered vehicle as carla_msgs/CarlaEgoVehicleStatus on
  /// <base_topic>/vehicle_status, once per frame. Acceleration is derived
  /// from the velocity of the previous frame, so the first frame after
  /// registration reports zero acceleration.
  class CarlaEgoVehicleStatusPublisher : public BasePublisher {
    public:
      struct StatusMsgTraits {
        using msg_type = msg::CarlaEgoVehicleStatus;
      };

      CarlaEgoVehicleStatusPublisher(std::string base_topic_name) :
        BasePublisher(base_topic_name + "/vehicle_status"),
        _impl(std::make_shared<PublisherImpl<StatusMsgTraits>>()) {
          if (!_impl->Init(GetBaseTopicName())) {
            log_warning("CarlaEgoVehicleStatusPublisher: Init failed for topic: ", GetBaseTopicName());
          }
      }

      bool Publish() {
        return _impl->Publish();
      }

      /// @param transform vehicle transform in UE coordinates (meters, degrees)
      /// @param linear_velocity world-frame velocity in m/s, UE coordinates
      /// @param delta_seconds simulation time elapsed since the previous frame
      /// @param control last control applied to the vehicle, echoed back
      bool Write(
          int32_t seconds,
          uint32_t nanoseconds,
          std::string frame_id,
          const geom::Transform &transform,
          const geom::Vector3D &linear_velocity,
          float delta_seconds,
          const rpc::VehicleControl &control);

      /// Returns (velocity - previous_velocity) / delta_seconds, or a zero
      /// vector when there is no previous sample or the delta is not positive.
      static geom::Vector3D ComputeAcceleration(
          const geom::Vector3D &velocity,
          const geom::Vector3D &previous_velocity,
          bool has_previous_velocity,
          float delta_seconds) {
        if (!has_previous_velocity || delta_seconds <= 0.0f) {
          return geom::Vector3D();
        }
        return (velocity - previous_velocity) / delta_seconds;
      }

    private:
      std::shared_ptr<PublisherImpl<StatusMsgTraits>> _impl;
      geom::Vector3D _previous_velocity;
      bool _has_previous_velocity{false};
  };

}  // namespace ros2
}  // namespace carla
