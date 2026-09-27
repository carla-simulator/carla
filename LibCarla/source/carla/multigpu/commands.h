// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <string_view>

namespace carla {
namespace multigpu {

enum MultiGPUCommand : uint32_t {
  SEND_FRAME = 0,
  LOAD_MAP,
  GET_TOKEN,
  ENABLE_ROS,
  DISABLE_ROS,
  IS_ENABLED_ROS,
  YOU_ALIVE
};

struct CommandHeader {
  MultiGPUCommand id;
  uint32_t size;
};

/// Sent unprompted by a secondary, on the same channel as command responses,
/// to signal that its episode is ready to receive frames (see
/// Router::HandleResponse). No command reply may ever equal this value.
inline constexpr std::string_view kEpisodeReadyMarker{"EPISODE_READY"};

} // namespace multigpu
} // namespace carla
