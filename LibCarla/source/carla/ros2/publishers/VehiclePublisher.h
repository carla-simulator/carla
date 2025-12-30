// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
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
#include "carla_msgs/msg/CarlaEgoVehicleStatusPubSubTypes.h"
#include "carla_msgs/msg/CarlaEgoVehicleInfoPubSubTypes.h"
#include "carla_msgs/msg/CarlaEgoVehicleTelemetryDataPubSubTypes.h"
#include "nav_msgs/msg/OdometryPubSubTypes.h"
#include "std_msgs/msg/Float32PubSubTypes.h"

namespace carla {
namespace ros2 {

using VehicleInfoPublisherImpl =
    DdsPublisherImpl<carla_msgs::msg::CarlaEgoVehicleInfo, carla_msgs::msg::CarlaEgoVehicleInfoPubSubType>;
using EgoVehicleStatusPublisherImpl =
    DdsPublisherImpl<carla_msgs::msg::CarlaEgoVehicleStatus, carla_msgs::msg::CarlaEgoVehicleStatusPubSubType>;
using VehicleSpeedPublisherImpl =
    DdsPublisherImpl<std_msgs::msg::Float32, std_msgs::msg::Float32PubSubType>;
using VehicleOdometryPublisherImpl = 
    DdsPublisherImpl<nav_msgs::msg::Odometry, nav_msgs::msg::OdometryPubSubType>;
using VehicleTelemetryDataPublisherImpl =
    DdsPublisherImpl<carla_msgs::msg::CarlaEgoVehicleTelemetryData, carla_msgs::msg::CarlaEgoVehicleTelemetryDataPubSubType>;


class VehiclePublisher : public PublisherBaseTransform {
public:
  VehiclePublisher(std::shared_ptr<carla::ros2::types::VehicleActorDefinition> vehicle_actor_definition,
                   std::shared_ptr<TransformPublisher> transform_publisher,
                   std::shared_ptr<ObjectsPublisher> objects_publisher,
                   std::shared_ptr<ObjectsWithCovariancePublisher> objects_with_covariance_publisher,
                   carla::rpc::RpcServerInterface &carla_server);
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

  /**
   * Perform message processing. 
   */
  bool ProcessMessages();

  void UpdateVehicle(std::shared_ptr<carla::ros2::types::Object> &object,
                     carla::sensor::data::ActorDynamicState const &actor_dynamic_state);

private:
  carla::rpc::RpcServerInterface &_carla_server;
  std::shared_ptr<VehicleInfoPublisherImpl> _vehicle_info_publisher;
  std::shared_ptr<EgoVehicleStatusPublisherImpl> _vehicle_status_publisher;
  std::shared_ptr<VehicleOdometryPublisherImpl> _vehicle_odometry_publisher;
  std::shared_ptr<VehicleSpeedPublisherImpl> _vehicle_speed_publisher;
  std::shared_ptr<VehicleTelemetryDataPublisherImpl> _vehicle_telemetry_publisher;
  std::shared_ptr<ObjectPublisher> _vehicle_object_publisher;
  std::shared_ptr<ObjectWithCovariancePublisher> _vehicle_object_with_covariance_publisher;
};
}  // namespace ros2
}  // namespace carla
