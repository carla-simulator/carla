// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/EpisodeSettings.h"
#include "carla_msgs/msg/CarlaEpisodeSettings.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Convert a carla EpisodeSettings to a ROS CarlaEpisodeSettings and vice-vera
*/
class EpisodeSettings {
public:
  explicit EpisodeSettings(carla::rpc::EpisodeSettings const& rpc_episode_settings = carla::rpc::EpisodeSettings()) {
    _ros_episode_settings.synchronous_mode(rpc_episode_settings.synchronous_mode);
    _ros_episode_settings.no_rendering_mode(rpc_episode_settings.no_rendering_mode);
    if ( rpc_episode_settings.fixed_delta_seconds.has_value() ) {
      _ros_episode_settings.fixed_delta_seconds(rpc_episode_settings.fixed_delta_seconds.value());
    }
    _ros_episode_settings.substepping(rpc_episode_settings.substepping);
    _ros_episode_settings.max_substep_delta_time(rpc_episode_settings.max_substep_delta_time);
    _ros_episode_settings.max_substeps(rpc_episode_settings.max_substeps);
    _ros_episode_settings.max_culling_distance(rpc_episode_settings.max_culling_distance);
    _ros_episode_settings.deterministic_ragdolls(rpc_episode_settings.deterministic_ragdolls);
    _ros_episode_settings.tile_stream_distance(rpc_episode_settings.tile_stream_distance);
    _ros_episode_settings.actor_active_distance(rpc_episode_settings.actor_active_distance);
    _ros_episode_settings.spectator_as_ego(rpc_episode_settings.spectator_as_ego);
  }
  
  explicit EpisodeSettings(carla_msgs::msg::CarlaEpisodeSettings const& carla_episode_settings) {
    _ros_episode_settings = carla_episode_settings;
  }
  ~EpisodeSettings() = default;
  EpisodeSettings(const EpisodeSettings&) = default;
  EpisodeSettings& operator=(const EpisodeSettings&) = default;
  EpisodeSettings(EpisodeSettings&&) = default;
  EpisodeSettings& operator=(EpisodeSettings&&) = default;

  /**
   * The resulting ROS carla_msgs::msg::CarlaEpisodeSettings
   */
  carla_msgs::msg::CarlaEpisodeSettings episode_settings() const {
    return _ros_episode_settings;
  }

  carla::rpc::EpisodeSettings GetEpisodeSettings() const {
    carla::rpc::EpisodeSettings episode_settings;
    episode_settings.synchronous_mode = _ros_episode_settings.synchronous_mode();
    episode_settings.no_rendering_mode = _ros_episode_settings.no_rendering_mode();
    episode_settings.fixed_delta_seconds = _ros_episode_settings.fixed_delta_seconds();
    episode_settings.substepping = _ros_episode_settings.substepping();
    episode_settings.max_substep_delta_time =  _ros_episode_settings.max_substep_delta_time();
    episode_settings.max_substeps = _ros_episode_settings.max_substeps();
    episode_settings.max_culling_distance = _ros_episode_settings.max_culling_distance();
    episode_settings.deterministic_ragdolls = _ros_episode_settings.deterministic_ragdolls();
    episode_settings.tile_stream_distance = _ros_episode_settings.tile_stream_distance();
    episode_settings.actor_active_distance = _ros_episode_settings.actor_active_distance();
    episode_settings.spectator_as_ego = _ros_episode_settings.spectator_as_ego();
    return episode_settings;
  }

private:
  carla_msgs::msg::CarlaEpisodeSettings _ros_episode_settings;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla