// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/CarlaActor.h"
#include "Carla/Sensor/DataStream.h"

#include "CoreMinimal.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/multigpu/router.h>
#include <carla/streaming/Server.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/ActorDescription.h>
#include <carla/rpc/AttachmentType.h>
#include <carla/rpc/Response.h>
#include <carla/rpc/Transform.h>
#include <carla/rpc/RpcServerInterface.h>
#include <compiler/enable-ue4-macros.h>

class UCarlaEpisode;

class FCarlaServer: public carla::rpc::RpcServerInterface
{
public:

  FCarlaServer();

  ~FCarlaServer();

  FDataMultiStream Start(uint16_t RPCPort, uint16_t StreamingPort, uint16_t SecondaryPort);

  void NotifyBeginEpisode(UCarlaEpisode &Episode);

  void NotifyEndEpisode();

  void AsyncRun(uint32 NumberOfWorkerThreads);

  void RunSome(uint32 Milliseconds);

  void SetROS2TopicVisibilityDefaultEnabled(bool _topic_visibility_default_enabled);

  void EnableSynchronousMode();
  void DisableSynchronousMode();
  bool IsSynchronousModeActive();

  double GetTickDeltaSeconds();
  void Tick();
  
  bool TickCueReceived();

  void Stop();

  FDataStream OpenStream() const;

  std::shared_ptr<carla::multigpu::Router> GetSecondaryServer();

  carla::streaming::Server &GetStreamingServer();

  /**
   * Reimplemented from ROS2ServerInterfase
  */
  std::shared_ptr<carla::streaming::detail::Dispatcher> GetDispatcher() override {
    return GetStreamingServer().GetDispatcher();
  }



  /**
   * @brief episode related calls
   * @{
   */
  carla::rpc::Response<void> call_load_new_episode(const std::string &map_name, const bool reset_settings, carla::rpc::MapLayer map_layers) override;
  carla::rpc::Response<carla::rpc::EpisodeSettings> call_get_episode_settings() override;
  carla::rpc::Response<uint64_t> call_set_episode_settings(carla::rpc::EpisodeSettings const &settings) override;
  /**
   * @}
   */

  /**
   * @brief map related calls
   * @{
   */
  carla::rpc::Response<std::vector<std::string>> call_get_available_maps() override;
  carla::rpc::Response<std::string> call_get_map_data() override;
  carla::rpc::Response<carla::rpc::MapInfo> call_get_map_info() override;
  /**
   * @}
   */

  /**
   * @brief actor related calls
   * @{
   */
  carla::rpc::Response<std::vector<carla::rpc::ActorDefinition> > call_get_actor_definitions() override;
  carla::rpc::Response<carla::rpc::Actor> call_spawn_actor(carla::rpc::ActorDescription Description, const carla::rpc::Transform &Transform) override;
  carla::rpc::Response<carla::rpc::Actor> call_spawn_actor_with_parent(carla::rpc::ActorDescription Description, const carla::rpc::Transform &Transform,
                                                       carla::rpc::ActorId ParentId, carla::rpc::AttachmentType InAttachmentType,
                                                       const std::string &socket_name) override;
  carla::rpc::Response<bool> call_destroy_actor(carla::rpc::ActorId ActorId) override;
  carla::rpc::Response<carla::rpc::VehicleTelemetryData> call_get_telemetry_data(carla::rpc::ActorId ActorId) override;
  /**
   * @}
   */

  /**
   * @brief ros actor interaction calls
   * @{
   */
  carla::rpc::Response<void> call_enable_actor_for_ros(carla::rpc::ActorId actor_id) override;
  carla::rpc::Response<void> call_disable_actor_for_ros(carla::rpc::ActorId actor_id) override;
  carla::rpc::Response<bool> call_is_actor_enabled_for_ros(carla::rpc::ActorId actor_id) override;
  /**
   * @}
   */

  /**
   * @brief synchronization calls
   * @{
   */
  carla::rpc::Response<uint64_t> call_tick(
      carla::rpc::synchronization_client_id_type const &client_id,
      carla::rpc::synchronization_participant_id_type const &participant_id,
      carla::rpc::SynchronizationTickMode synchronization_tick_mode) override;
  carla::rpc::Response<carla::rpc::synchronization_participant_id_type> call_register_synchronization_participant(
      carla::rpc::synchronization_client_id_type const &client_id,
      carla::rpc::synchronization_participant_id_type const &participant_id_hint = carla::rpc::ALL_PARTICIPANTS) override;
  carla::rpc::Response<bool> call_deregister_synchronization_participant(
      carla::rpc::synchronization_client_id_type const &client_id, carla::rpc::synchronization_participant_id_type const &participant_id) override;
  carla::rpc::Response<bool> call_update_synchronization_window(
      carla::rpc::synchronization_client_id_type const &client_id, carla::rpc::synchronization_participant_id_type const &participant_id,
      carla::rpc::synchronization_target_game_time const &target_game_time = carla::rpc::NO_SYNC_TARGET_GAME_TIME) override;
  carla::rpc::Response<std::pair< bool , std::vector<carla::rpc::synchronization_window_participant_state> > >  call_get_synchronization_window_status() override;
  /**
   * @}
   */
private:

  class FPimpl;
  TUniquePtr<FPimpl> Pimpl;
};
