// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
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
#include <carla/ros2/ROS2ServerInterface.h>
#include <compiler/enable-ue4-macros.h>

class UCarlaEpisode;

class FCarlaServer: public carla::ros2::ROS2ServerInterface
{
public:

  FCarlaServer();

  ~FCarlaServer();

  FDataMultiStream Start(uint16_t RPCPort, uint16_t StreamingPort, uint16_t SecondaryPort);

  void NotifyBeginEpisode(UCarlaEpisode &Episode);

  void NotifyEndEpisode();

  void AsyncRun(uint32 NumberOfWorkerThreads);

  void RunSome(uint32 Milliseconds);

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

  carla::rpc::Response<std::vector<carla::rpc::ActorDefinition> > call_get_actor_definitions() override;
  carla::rpc::Response<carla::rpc::MapInfo> call_get_map_info() override;
  carla::rpc::Response<std::string> call_get_map_data() override;
  carla::rpc::Response<carla::rpc::Actor> call_spawn_actor(carla::rpc::ActorDescription Description, const carla::rpc::Transform &Transform) override;
  carla::rpc::Response<carla::rpc::Actor> call_spawn_actor_with_parent(
    carla::rpc::ActorDescription Description,
    const carla::rpc::Transform &Transform,
    carla::rpc::ActorId ParentId,
    carla::rpc::AttachmentType InAttachmentType,
    const std::string& socket_name) override;
  carla::rpc::Response<bool> call_destroy_actor(carla::rpc::ActorId ActorId) override;

private:

  class FPimpl;
  TUniquePtr<FPimpl> Pimpl;
};
