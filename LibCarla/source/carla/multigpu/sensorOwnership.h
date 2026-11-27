// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace multigpu {

/// Whether the primary publishes a sensor on ROS 2.
constexpr bool PrimaryOwnsSensor(bool primary_only_sensor, bool routed_to_live_secondary) {
  return primary_only_sensor || !routed_to_live_secondary;
}

/// Whether a secondary publishes a sensor on ROS 2.
constexpr bool SecondaryOwnsSensor(bool primary_only_sensor, bool connected_to_primary, bool routed_here) {
  return !primary_only_sensor && connected_to_primary && routed_here;
}

/// Whether a sensor must produce data in this process.
constexpr bool ShouldProduceSensorData(
    bool owns_sensor,
    bool has_client_sessions,
    bool has_any_listener) {
  return has_client_sessions || (owns_sensor && has_any_listener);
}

} // namespace multigpu
} // namespace carla
