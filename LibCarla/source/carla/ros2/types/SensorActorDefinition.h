// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/types/ActorNameDefinition.h"
#include "carla/ros2/types/PublisherSensorType.h"
#include "carla/streaming/detail/Types.h"

namespace carla {
namespace ros2 {
namespace types {

struct SensorActorDefinition : public ActorNameDefinition {
  SensorActorDefinition(ActorNameDefinition const &actor_name_definition,
                        carla::ros2::types::PublisherSensorType sensor_type_,
                        carla::streaming::detail::stream_id_type stream_id_)
    : ActorNameDefinition(actor_name_definition), sensor_type(sensor_type_), stream_id(stream_id_) {}

  carla::ros2::types::PublisherSensorType sensor_type;
  carla::streaming::detail::stream_id_type stream_id;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla

namespace std {

inline std::string to_string(carla::ros2::types::SensorActorDefinition const &actor_definition) {
  return "SensorActor(" + to_string(static_cast<carla::ros2::types::ActorNameDefinition>(actor_definition)) +
         " sensor_type=" + std::to_string(actor_definition.sensor_type) +
         " stream_id=" + std::to_string(actor_definition.stream_id) + ")";
}

}  // namespace std