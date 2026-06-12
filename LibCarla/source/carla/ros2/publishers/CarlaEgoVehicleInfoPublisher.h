// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "carla/geom/Transform.h"
#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/rpc/VehiclePhysicsControl.h"

#include "carla/ros2/types/msg/CarlaEgoVehicleInfo.h"

namespace carla {
namespace ros2 {

  /// Publishes the static description of a registered vehicle as a latched
  /// carla_msgs/CarlaEgoVehicleInfo on <base_topic>/vehicle_info, once at
  /// registration. The transient_local durability lets late-joining
  /// subscribers receive it without CARLA re-publishing.
  class CarlaEgoVehicleInfoPublisher : public BasePublisher {
    public:
      struct InfoMsgTraits {
        using msg_type = msg::CarlaEgoVehicleInfo;
      };

      CarlaEgoVehicleInfoPublisher(std::string base_topic_name) :
        BasePublisher(base_topic_name + "/vehicle_info"),
        _impl(std::make_shared<PublisherImpl<InfoMsgTraits>>()) {
          PublisherQos qos;
          qos.durability = DurabilityKind::TransientLocal;
          if (!_impl->Init(GetBaseTopicName(), qos)) {
            log_warning("CarlaEgoVehicleInfoPublisher: Init failed for topic: ", GetBaseTopicName());
          }
      }

      bool Publish() {
        return _impl->Publish();
      }

      /// @param vehicle_transform vehicle transform in UE coordinates (meters,
      ///        degrees), used to express wheel positions in the vehicle frame
      /// @param physics_control physics description as reported by the simulator
      bool Write(
          uint32_t id,
          const std::string &type_id,
          const std::string &role_name,
          const geom::Transform &vehicle_transform,
          const rpc::VehiclePhysicsControl &physics_control);

    private:
      std::shared_ptr<PublisherImpl<InfoMsgTraits>> _impl;
  };

}  // namespace ros2
}  // namespace carla
