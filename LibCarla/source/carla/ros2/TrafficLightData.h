// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"

namespace carla {
namespace ros2 {

  /// Snapshot of one traffic light's state, gathered by the simulator each
  /// frame. The state encoding follows carla_msgs/CarlaTrafficLightStatus:
  /// RED=0, YELLOW=1, GREEN=2, OFF=3, UNKNOWN=4 (same ordinals as
  /// carla::rpc::TrafficLightState).
  struct TrafficLightState {
    uint32_t id = 0u;
    uint8_t state = 0u;
  };

  /// Static description of one traffic light, gathered once per episode.
  /// All values use UE coordinates in meters; the ROS conversion happens in
  /// the publisher.
  struct TrafficLightInfo {
    uint32_t id = 0u;
    /// World pose of the light.
    carla::geom::Transform transform;
    /// Trigger volume center, relative to the light transform.
    carla::geom::Location trigger_center;
    /// Trigger volume half-extent.
    carla::geom::Vector3D trigger_extent;
  };

} // namespace ros2
} // namespace carla
