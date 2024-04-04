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
  virtual carla::rpc::Response<std::vector<carla::rpc::ActorDefinition> > call_get_actor_definitions() = 0;
  virtual carla::rpc::Response<carla::rpc::EpisodeSettings> call_get_episode_settings() = 0;
  virtual carla::rpc::Response<uint64_t> call_set_episode_settings(carla::rpc::EpisodeSettings const &settings) = 0;
  virtual carla::rpc::Response<carla::rpc::MapInfo> call_get_map_info() = 0;
  virtual carla::rpc::Response<std::string> call_get_map_data() = 0;
  virtual carla::rpc::Response<carla::rpc::Actor> call_spawn_actor(carla::rpc::ActorDescription Description,
                                                                   const carla::rpc::Transform &Transform) = 0;
  virtual carla::rpc::Response<carla::rpc::Actor> call_spawn_actor_with_parent(
      carla::rpc::ActorDescription Description, const carla::rpc::Transform &Transform, carla::rpc::ActorId ParentId,
      carla::rpc::AttachmentType InAttachmentType, const std::string &socket_name) = 0;
  virtual carla::rpc::Response<bool> call_destroy_actor(carla::rpc::ActorId ActorId) = 0;
};

}  // namespace rpc
}  // namespace carla