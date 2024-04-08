// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/ObjectPublisher.h"
#include "carla/ros2/publishers/PublisherBaseTransform.h"
#include "carla/ros2/types/Object.h"
#include "carla/ros2/types/Transform.h"
#include "carla/ros2/types/VehicleActorDefinition.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/sensor/data/ActorDynamicState.h"
#include "carla_msgs/msg/CarlaVehicleInfoPubSubTypes.h"
#include "carla_msgs/msg/CarlaVehicleStatusPubSubTypes.h"

namespace carla {
namespace ros2 {

using VehicleInfoPublisherImpl =
    DdsPublisherImpl<carla_msgs::msg::CarlaVehicleInfo, carla_msgs::msg::CarlaVehicleInfoPubSubType>;
using VehicleStatusPublisherImpl =
    DdsPublisherImpl<carla_msgs::msg::CarlaVehicleStatus, carla_msgs::msg::CarlaVehicleStatusPubSubType>;

class VehiclePublisher : public PublisherBaseTransform {
public:
  VehiclePublisher(std::shared_ptr<carla::ros2::types::VehicleActorDefinition> vehicle_actor_definition,
                   std::shared_ptr<TransformPublisher> transform_publisher,
                   std::shared_ptr<ObjectsPublisher> objects_publisher);
  virtual ~VehiclePublisher() = default;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;
  /**
   * Implement PublisherInterface::SubscribersConnected interface
   */
  bool SubscribersConnected() const override;

  void UpdateVehicle(std::shared_ptr<carla::ros2::types::Object> &object,
                     carla::sensor::data::ActorDynamicState const &actor_dynamic_state);

private:
  std::shared_ptr<VehicleInfoPublisherImpl> _vehicle_info;
  bool _vehicle_info_published{false};
  std::shared_ptr<VehicleStatusPublisherImpl> _vehicle_status;
  std::shared_ptr<ObjectPublisher> _vehicle_object_publisher;
};
}  // namespace ros2
}  // namespace carla
