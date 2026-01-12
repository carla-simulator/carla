// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/ObjectPublisher.h"
#include "carla/ros2/publishers/PublisherBase.h"
#include "carla/ros2/types/Object.h"
#include "carla/ros2/types/TrafficSignActorDefinition.h"
#include "carla/sensor/data/ActorDynamicState.h"

#define PUBLISH_INDIVIDUAL_TRAFFIC_SIGN_DATA  0

namespace carla {
namespace ros2 {

class TrafficSignPublisher : public PublisherBase {
public:
  TrafficSignPublisher(std::shared_ptr<carla::ros2::types::TrafficSignActorDefinition> traffic_sign_actor_definition,
                       std::shared_ptr<ObjectsPublisher> objects_publisher);
  virtual ~TrafficSignPublisher() = default;

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

  void UpdateTrafficSign(std::shared_ptr<const carla::ros2::types::Object> &object,
                         carla::sensor::data::ActorDynamicState const &actor_dynamic_state);

private:
#if PUBLISH_INDIVIDUAL_TRAFFIC_SIGN_DATA
  std::shared_ptr<ObjectPublisher> _traffic_sign_object_publisher;
#else
  std::shared_ptr<ObjectsPublisher> _traffic_sign_objects_publisher;
#endif
};
}  // namespace ros2
}  // namespace carla
