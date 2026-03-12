// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/ObjectPublisher.h"
#include "carla/ros2/publishers/PublisherBase.h"
#include "carla/ros2/publishers/TrafficLightsPublisher.h"
#include "carla/ros2/types/Object.h"
#include "carla/ros2/types/TrafficLightActorDefinition.h"
#include "carla/sensor/data/ActorDynamicState.h"
#include "carla_msgs/msg/CarlaTrafficLightInfoPubSubTypes.h"
#include "carla_msgs/msg/CarlaTrafficLightStatusPubSubTypes.h"

#define PUBLISH_INDIVIDUAL_TRAFFIC_LIGHT_DATA  0

namespace carla {
namespace ros2 {

using TrafficLightInfoPublisherImpl =
    DdsPublisherImpl<carla_msgs::msg::CarlaTrafficLightInfo, carla_msgs::msg::CarlaTrafficLightInfoPubSubType>;
using TrafficLightStatusPublisherImpl =
    DdsPublisherImpl<carla_msgs::msg::CarlaTrafficLightStatus, carla_msgs::msg::CarlaTrafficLightStatusPubSubType>;

class TrafficLightPublisher : public PublisherBase {
public:
  TrafficLightPublisher(std::shared_ptr<carla::ros2::types::TrafficLightActorDefinition> traffic_light_actor_definition,
                        std::shared_ptr<ObjectsPublisher> objects_publisher,
                        std::shared_ptr<TrafficLightsPublisher> traffic_lights_publisher);
  virtual ~TrafficLightPublisher() = default;

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

  void UpdateTrafficLight(std::shared_ptr<const carla::ros2::types::Object> &object,
                          carla::sensor::data::ActorDynamicState const &actor_dynamic_state);

private:
  carla_msgs::msg::CarlaTrafficLightStatus _traffic_light_status;
  carla_msgs::msg::CarlaTrafficLightInfo _traffic_light_info;
  bool _traffic_light_info_initialized{false};
#if PUBLISH_INDIVIDUAL_TRAFFIC_LIGHT_DATA
  std::shared_ptr<TrafficLightInfoPublisherImpl> _traffic_light_info_publisher;
  bool _traffic_light_info_published{false};
  std::shared_ptr<TrafficLightStatusPublisherImpl> _traffic_light_status_publisher;
  std::shared_ptr<ObjectPublisher> _traffic_light_object_publisher;
#else
  std::shared_ptr<ObjectsPublisher> _traffic_light_objects_publisher;
#endif
  std::shared_ptr<TrafficLightsPublisher> _traffic_lights_publisher;
};
}  // namespace ros2
}  // namespace carla
