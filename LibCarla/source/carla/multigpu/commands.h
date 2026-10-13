// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
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
  YOU_ALIVE,
  SET_PUBLISH_TF
};

struct CommandHeader {
  MultiGPUCommand id;
  uint32_t size;
};

/// Sent unprompted by a secondary, on the same channel as command responses,
/// to signal that its episode is ready to receive frames (see
/// Router::HandleResponse). No command reply may ever start with this value.
inline constexpr std::string_view kEpisodeReadyMarker{"EPISODE_READY"};

/// Identifies one LOAD_MAP broadcast. Assigned by the primary starting at 1;
/// 0 means the episode was not started by a LOAD_MAP.
using load_map_id_type = uint32_t;

/// LOAD_MAP payload: the NUL-terminated map path followed by the load id.
inline std::string MakeLoadMapPayload(std::string_view map, load_map_id_type load_id) {
  std::string payload(map);
  payload.push_back('\0');
  payload.append(reinterpret_cast<const char *>(&load_id), sizeof(load_id));
  return payload;
}

struct LoadMapRequest {
  std::string map;
  load_map_id_type load_id = 0u;
};

inline LoadMapRequest ParseLoadMapPayload(std::string_view payload) {
  LoadMapRequest request;
  const auto end_of_map = payload.find('\0');
  request.map = std::string(payload.substr(0u, end_of_map));
  if (
      (end_of_map != std::string_view::npos) &&
      (payload.size() - end_of_map - 1u >= sizeof(load_map_id_type))) {
    std::memcpy(&request.load_id, payload.data() + end_of_map + 1u, sizeof(request.load_id));
  }
  return request;
}

/// Episode-ready message: the marker followed by the load id of the LOAD_MAP
/// that started the episode, so the primary can ignore a stale one.
inline std::string MakeEpisodeReadyMessage(load_map_id_type load_id) {
  std::string message(kEpisodeReadyMarker);
  message.append(reinterpret_cast<const char *>(&load_id), sizeof(load_id));
  return message;
}

/// Load id of an episode-ready message (0 for a bare marker), or nullopt if
/// @a payload is not one.
inline std::optional<load_map_id_type> ParseEpisodeReadyMessage(std::string_view payload) {
  if (payload.substr(0u, kEpisodeReadyMarker.size()) != kEpisodeReadyMarker) {
    return std::nullopt;
  }
  const auto load_id_bytes = payload.substr(kEpisodeReadyMarker.size());
  if (load_id_bytes.empty()) {
    return load_map_id_type{0u};
  }
  if (load_id_bytes.size() != sizeof(load_map_id_type)) {
    return std::nullopt;
  }
  load_map_id_type load_id = 0u;
  std::memcpy(&load_id, load_id_bytes.data(), sizeof(load_id));
  return load_id;
}

/// GET_TOKEN request payload. The secondary resolves the sensor by
/// @a actor_id (the primary's actor id) and serves it under @a stream_id (the
/// primary's stream id), which stays the only stream id clients ever see.
struct GetTokenRequest {
  uint32_t stream_id;
  uint32_t actor_id;
};
static_assert(sizeof(GetTokenRequest) == 2 * sizeof(uint32_t));

/// GET_TOKEN reply when the secondary cannot resolve the sensor to a stream
/// (or the request is malformed). Shorter than a token on purpose.
inline constexpr std::string_view kTokenNotReadyMarker{"TOKEN_NOT_READY"};

} // namespace multigpu
} // namespace carla
