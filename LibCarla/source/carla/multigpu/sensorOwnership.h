// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace multigpu {

enum class ProcessRole {
  Standalone,
  Primary,
  Secondary
};

/// Whether this process captures and publishes a sensor. The primary keeps
/// every sensor not routed to a live secondary, and its primary-only sensors
/// (collision, vehicle status) always; a secondary owns a sensor once the
/// primary routed its token there.
[[nodiscard]]
constexpr bool OwnsSensor(
    ProcessRole role,
    bool routed_to_secondary,
    bool primary_only_sensor,
    bool token_granted) {
  switch (role) {
    case ProcessRole::Primary:
      return primary_only_sensor || !routed_to_secondary;
    case ProcessRole::Secondary:
      return !primary_only_sensor && token_granted;
    case ProcessRole::Standalone:
      break;
  }
  return true;
}

} // namespace multigpu
} // namespace carla
