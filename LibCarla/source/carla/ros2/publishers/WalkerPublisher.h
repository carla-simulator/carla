// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/ObjectPublisher.h"
#include "carla/ros2/publishers/PublisherBaseTransform.h"
#include "carla/ros2/types/Object.h"
#include "carla/ros2/types/Transform.h"
#include "carla/ros2/types/WalkerActorDefinition.h"
#include "carla/sensor/data/ActorDynamicState.h"

namespace carla {
namespace ros2 {

class WalkerPublisher : public PublisherBaseTransform {
public:
  WalkerPublisher(std::shared_ptr<carla::ros2::types::WalkerActorDefinition> walker_actor_definition,
                  std::shared_ptr<TransformPublisher> transform_publisher,
                  std::shared_ptr<ObjectsPublisher> objects_publisher);
  virtual ~WalkerPublisher() = default;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;
  /**
   * Implement PublisherInterface::SubsribersConnected interface
   */
  bool SubsribersConnected() const override;

  void UpdateWalker(std::shared_ptr<carla::ros2::types::Object> &object,
                    carla::sensor::data::ActorDynamicState const &actor_dynamic_state);

private:
  std::shared_ptr<ObjectPublisher> _walker_object_publisher;
};
}  // namespace ros2
}  // namespace carla
