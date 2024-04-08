// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/Actor.h"
#include "carla/rpc/ActorDefinition.h"
#include "carla/rpc/ActorDescription.h"
#include "carla/rpc/AttachmentType.h"
#include "carla/rpc/EpisodeSettings.h"
#include "carla/rpc/MapInfo.h"
#include "carla/rpc/Response.h"
#include "carla/rpc/ServerSynchronizationTypes.h"
#include "carla/rpc/Transform.h"
#include "carla/streaming/detail/Dispatcher.h"

namespace carla {
namespace rpc {

/// The interface to the CARLA server required from TCP and ROS2 client side.
/// The parts only required from TPC client side are handled by lambdas directly.
class RpcServerInterface {
public:
  RpcServerInterface() = default;
  virtual ~RpcServerInterface() = default;

  // Server functions to be called also from ROS2 interface.
  // Those have to be explicitly callable functions instead of lambdas
  virtual std::shared_ptr<carla::streaming::detail::Dispatcher> GetDispatcher() = 0;
  virtual Response<std::vector<ActorDefinition> > call_get_actor_definitions() = 0;
  virtual Response<EpisodeSettings> call_get_episode_settings() = 0;
  virtual Response<uint64_t> call_set_episode_settings(EpisodeSettings const &settings) = 0;
  virtual Response<MapInfo> call_get_map_info() = 0;
  virtual Response<std::string> call_get_map_data() = 0;

  /**
   * @brief spawn/destroy actor calls
   * @{
   */
  virtual Response<Actor> call_spawn_actor(ActorDescription Description, const Transform &Transform) = 0;
  virtual Response<Actor> call_spawn_actor_with_parent(ActorDescription Description, const Transform &Transform,
                                                       ActorId ParentId, AttachmentType InAttachmentType,
                                                       const std::string &socket_name) = 0;
  virtual Response<bool> call_destroy_actor(ActorId ActorId) = 0;
  /**
   * @}
   */

  /**
   * @brief synchronization calls
   * @{
   */
  virtual Response<uint64_t> call_tick(
      synchronization_client_id_type const &client_id = ALL_CLIENTS,
      synchronization_participant_id_type const &participant_id = ALL_PARTICIPANTS) = 0;
  virtual Response<synchronization_participant_id_type> call_register_synchronization_participant(
      synchronization_client_id_type const &client_id,
      synchronization_participant_id_type const &participant_id_hint = ALL_PARTICIPANTS) = 0;
  virtual Response<bool> call_deregister_synchronization_participant(
      synchronization_client_id_type const &client_id, synchronization_participant_id_type const &participant_id) = 0;
  virtual Response<bool> call_update_synchronization_window(
      synchronization_client_id_type const &client_id, synchronization_participant_id_type const &participant_id,
      synchronization_target_game_time const &target_game_time = NO_SYNC_TARGET_GAME_TIME) = 0;
  /**
   * @}
   */
};

}  // namespace rpc
}  // namespace carla
