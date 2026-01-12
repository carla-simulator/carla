// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/PublisherBase.h"
#include "carla/ros2/types/SensorActorDefinition.h"

namespace carla {
namespace ros2 {

/**
  A Publisher base class for publisher that provide data similiar or equal to sensors.
  Extends PublisherBase by specialized sensor get_topic_qos().
*/
class PublisherBaseSensor : public PublisherBase {
public:
  PublisherBaseSensor(std::shared_ptr<carla::ros2::types::ActorNameDefinition> actor_name_definition)
    : PublisherBase(actor_name_definition) {}
  virtual ~PublisherBaseSensor() = default;
};
}  // namespace ros2
}  // namespace carla
