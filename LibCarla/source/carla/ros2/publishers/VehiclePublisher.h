// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/ObjectPublisher.h"
#include "carla/ros2/publishers/ObjectWithCovariancePublisher.h"
#include "carla/ros2/publishers/PublisherBaseTransform.h"
#include "carla/ros2/types/Object.h"
#include "carla/ros2/types/Transform.h"
#include "carla/ros2/types/VehicleActorDefinition.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/rpc/RpcServerInterface.h"
#include "carla/sensor/data/ActorDynamicState.h"
#include "carla_msgs/msg/CarlaVehicleControlStatusPubSubTypes.h"
#include "carla_msgs/msg/CarlaVehicleInfoPubSubTypes.h"
#include "carla_msgs/msg/CarlaVehicleTelemetryDataPubSubTypes.h"
#include "nav_msgs/msg/OdometryPubSubTypes.h"
#include "std_msgs/msg/Float32PubSubTypes.h"

namespace carla {
namespace ros2 {

using VehicleInfoPublisherImpl =
    DdsPublisherImpl<carla_msgs::msg::CarlaVehicleInfo, carla_msgs::msg::CarlaVehicleInfoPubSubType>;
using VehicleControlStatusPublisherImpl =
    DdsPublisherImpl<carla_msgs::msg::CarlaVehicleControlStatus, carla_msgs::msg::CarlaVehicleControlStatusPubSubType>;
using VehicleSpeedPublisherImpl =
    DdsPublisherImpl<std_msgs::msg::Float32, std_msgs::msg::Float32PubSubType>;
using VehicleOdometryPublisherImpl = 
    DdsPublisherImpl<nav_msgs::msg::Odometry, nav_msgs::msg::OdometryPubSubType>;
using VehicleTelemetryDataPublisherImpl =
    DdsPublisherImpl<carla_msgs::msg::CarlaVehicleTelemetryData, carla_msgs::msg::CarlaVehicleTelemetryDataPubSubType>;


class VehiclePublisher : public PublisherBaseTransform {
public:
  VehiclePublisher(std::shared_ptr<carla::ros2::types::VehicleActorDefinition> vehicle_actor_definition,
                   std::shared_ptr<TransformPublisher> transform_publisher,
                   std::shared_ptr<ObjectsPublisher> objects_publisher,
                   std::shared_ptr<ObjectsWithCovariancePublisher> objects_with_covariance_publisher);
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
                     carla::sensor::data::ActorDynamicState const &actor_dynamic_state,
                     carla::rpc::RpcServerInterface &carla_server);

private:
  std::shared_ptr<VehicleInfoPublisherImpl> _vehicle_info_publisher;
  bool _vehicle_info_published{false};
  std::shared_ptr<VehicleControlStatusPublisherImpl> _vehicle_control_status_publisher;
  std::shared_ptr<VehicleOdometryPublisherImpl> _vehicle_odometry_publisher;
  std::shared_ptr<VehicleSpeedPublisherImpl> _vehicle_speed_publisher;
  std::shared_ptr<VehicleTelemetryDataPublisherImpl> _vehicle_telemetry_publisher;
  std::shared_ptr<ObjectPublisher> _vehicle_object_publisher;
  std::shared_ptr<ObjectWithCovariancePublisher> _vehicle_object_with_covariance_publisher;
};
}  // namespace ros2
}  // namespace carla
