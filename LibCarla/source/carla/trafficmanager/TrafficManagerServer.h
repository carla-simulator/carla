// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <vector>

#include "carla/Exception.h"
#include "carla/client/Actor.h"
#include "carla/client/detail/ActorVariant.h"
#include "carla/rpc/Server.h"
#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/TrafficManagerBase.h"

namespace carla {
namespace traffic_manager {

using ActorPtr = carla::SharedPtr<carla::client::Actor>;
using Path = std::vector<cg::Location>;
using Route = std::vector<uint8_t>;

using namespace constants::Networking;

class TrafficManagerServer {
public:

  TrafficManagerServer(const TrafficManagerServer &) = default;
  TrafficManagerServer(TrafficManagerServer &&) = default;

  TrafficManagerServer &operator=(const TrafficManagerServer &) = default;
  TrafficManagerServer &operator=(TrafficManagerServer &&) = default;

  /// Here RPCPort is the traffic manager local instance RPC server port where
  /// it can listen to remote traffic managers and apply the changes to
  /// local instance through a TrafficManagerBase pointer.
  TrafficManagerServer(
      uint16_t &RPCPort,
      carla::traffic_manager::TrafficManagerBase* tm)
    : _RPCPort(RPCPort) {

    uint16_t counter = 0;
    while(counter < MIN_TRY_COUNT) {
      try {

        /// Create server instance.
        server = new ::rpc::server(RPCPort);

      } catch(std::exception) {
        using namespace std::chrono_literals;
        /// Update port number and try again.
        std::this_thread::sleep_for(500ms);
      }

      /// If server created.
      if(server != nullptr) {
        break;
      }
      counter ++;
    }

    /// If server still not created throw a runtime exception.
    if(server == nullptr) {

      carla::throw_exception(std::runtime_error(
        "trying to create rpc server for traffic manager; "
        "but the system failed to create because of bind error."));
    } else {
      /// If the server creation was successful we are
      /// binding a lambda function to the name "register_vehicle".
      server->bind("register_vehicle", [=](std :: vector <carla::rpc::Actor> _actor_list) {
        std::vector<ActorPtr> actor_list;
        for (auto &&actor : _actor_list) {
          actor_list.emplace_back(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()));
        }
        tm->RegisterVehicles(actor_list);
      });


      /// Binding a lambda function to the name "unregister_vehicle".
      server->bind("unregister_vehicle", [=](std :: vector <carla::rpc::Actor> _actor_list) {
        std::vector<ActorPtr> actor_list;
        for (auto &&actor : _actor_list) {
          actor_list.emplace_back(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()));
        }
        tm->UnregisterVehicles(actor_list);
      });

      /// Method to set a vehicle's % decrease in velocity with respect to the speed limit.
      /// If less than 0, it's a % increase.
      server->bind("set_percentage_speed_difference", [=](carla::rpc::Actor actor, const float percentage) {
        tm->SetPercentageSpeedDifference(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), percentage);
      });

      /// Method to set a lane offset displacement from the center line.
      /// Positive values imply a right offset while negative ones mean a left one.
      server->bind("set_lane_offset", [=](carla::rpc::Actor actor, const float offset) {
        tm->SetLaneOffset(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), offset);
      });

      /// Set a vehicle's exact desired velocity.
      server->bind("set_desired_speed", [=](carla::rpc::Actor actor, const float value) {
        tm->SetDesiredSpeed(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), value);
      });

      /// Method to set the automatic management of the vehicle lights
      server->bind("update_vehicle_lights", [=](carla::rpc::Actor actor, const bool do_update) {
        tm->SetUpdateVehicleLights(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), do_update);
      });

      /// Method to set a global % decrease in velocity with respect to the speed limit.
      /// If less than 0, it's a % increase.
      server->bind("set_global_percentage_speed_difference", [=](const float percentage) {
        tm->SetGlobalPercentageSpeedDifference(percentage);
      });

      /// Method to set a global lane offset displacement from the center line.
      /// Positive values imply a right offset while negative ones mean a left one.
      server->bind("set_global_lane_offset", [=](const float offset) {
        tm->SetGlobalLaneOffset(offset);
      });


      /// Method to set collision detection rules between vehicles.
      server->bind("set_collision_detection", [=](const carla::rpc::Actor &reference_actor, const carla::rpc::Actor &other_actor, const bool detect_collision) {
        const auto reference = carla::client::detail::ActorVariant(reference_actor).Get(tm->GetEpisodeProxy());
        const auto other = carla::client::detail::ActorVariant(other_actor).Get(tm->GetEpisodeProxy());
        tm->SetCollisionDetection(reference, other, detect_collision);
      });

      /// Method to force lane change on a vehicle.
      /// Direction flag can be set to true for left and false for right.
      server->bind("set_force_lane_change", [=](carla::rpc::Actor actor, const bool direction) {
        tm->SetForceLaneChange(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), direction);
      });

     /// Enable/disable automatic lane change on a vehicle.
      server->bind("set_auto_lane_change", [=](carla::rpc::Actor actor, const bool enable) {
        tm->SetAutoLaneChange(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), enable);
      });

      /// Method to specify how much distance a vehicle should maintain to
      /// the leading vehicle.
      server->bind("set_distance_to_leading_vehicle", [=](carla::rpc::Actor actor, const float distance) {
        tm->SetDistanceToLeadingVehicle(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), distance);
      });

      /// Method to the Global Distance to Leading vehicle

      server->bind("set_global_distance_to_leading_vehicle", [=]( const float distance) {
        tm->SetGlobalDistanceToLeadingVehicle(distance);
      });

      /// Method to specify the % chance of running any traffic light.
      server->bind("set_percentage_running_light", [=](carla::rpc::Actor actor, const float percentage) {
        tm->SetPercentageRunningLight(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), percentage);
      });

      /// Method to specify the % chance of running any traffic sign.
      server->bind("set_percentage_running_sign", [=](carla::rpc::Actor actor, const float percentage) {
        tm->SetPercentageRunningSign(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), percentage);
      });

      /// Method to specify the % chance of ignoring collisions with any walker.
      server->bind("set_percentage_ignore_walkers", [=](carla::rpc::Actor actor, const float percentage) {
        tm->SetPercentageIgnoreWalkers(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), percentage);
      });

      /// Method to specify the % chance of ignoring collisions with any vehicle.
      server->bind("set_percentage_ignore_vehicles", [=](carla::rpc::Actor actor, const float percentage) {
        tm->SetPercentageIgnoreVehicles(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), percentage);
      });

      /// Method to set % to keep on the right lane.
      server->bind("set_percentage_keep_right_rule", [=](carla::rpc::Actor actor, const float percentage) {
        tm->SetKeepRightPercentage(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), percentage);
      });

      /// Method to set % to randomly do a left lane change.
      server->bind("set_percentage_random_left_lanechange", [=](carla::rpc::Actor actor, const float percentage) {
        tm->SetRandomLeftLaneChangePercentage(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), percentage);
      });

      /// Method to set % to randomly do a right lane change.
      server->bind("set_percentage_random_right_lanechange", [=](carla::rpc::Actor actor, const float percentage) {
        tm->SetRandomRightLaneChangePercentage(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), percentage);
      });

      /// Method to set hybrid physics mode.
      server->bind("set_hybrid_physics_mode", [=](const bool mode_switch) {
        tm->SetHybridPhysicsMode(mode_switch);
      });

      /// Method to set hybrid physics radius.
      server->bind("set_hybrid_physics_radius", [=](const float radius) {
        tm->SetHybridPhysicsRadius(radius);
      });

      /// Method to set hybrid physics radius.
      server->bind("set_osm_mode", [=](const bool mode_switch) {
        tm->SetOSMMode(mode_switch);
      });

      /// Method to set our own imported path.
      server->bind("set_path", [=](carla::rpc::Actor actor, const Path path, const bool empty_buffer) {
        tm->SetCustomPath(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), path, empty_buffer);
      });

      /// Method to remove a list of points.
      server->bind("remove_custom_path", [=](const ActorId actor_id, const bool remove_path) {
        tm->RemoveUploadPath(actor_id, remove_path);
      });

       /// Method to update an already set list of points.
      server->bind("update_custom_path", [=](const ActorId actor_id, const Path path) {
        tm->UpdateUploadPath(actor_id, path);
      });

      /// Method to set our own imported route.
      server->bind("set_imported_route", [=](carla::rpc::Actor actor, const Route route, const bool empty_buffer) {
        tm->SetImportedRoute(carla::client::detail::ActorVariant(actor).Get(tm->GetEpisodeProxy()), route, empty_buffer);
      });

      /// Method to remove a route.
      server->bind("remove_imported_route", [=](const ActorId actor_id, const bool remove_path) {
        tm->RemoveImportedRoute(actor_id, remove_path);
      });

      /// Method to update an already set list of points.
      server->bind("update_imported_route", [=](const ActorId actor_id, const Route route) {
        tm->UpdateImportedRoute(actor_id, route);
      });

      /// Method to set respawn dormant vehicles mode.
      server->bind("set_respawn_dormant_vehicles", [=](const bool mode_switch) {
        tm->SetRespawnDormantVehicles(mode_switch);
      });

      /// Method to set respawn dormant vehicles mode.
      server->bind("set_boundaries_respawn_dormant_vehicles", [=](const float lower_bound, const float upper_bound) {
        tm->SetBoundariesRespawnDormantVehicles(lower_bound, upper_bound);
      });

      /// Method to get the vehicle's next action.
      server->bind("get_next_action", [=](const ActorId actor_id) {
        tm->GetNextAction(actor_id);
      });

      /// Method to get the vehicle's action buffer.
      server->bind("get_all_actions", [=](const ActorId actor_id) {
        tm->GetActionBuffer(actor_id);
      });

      server->bind("shut_down", [=]() {
        tm->Release();
      });

      /// Method to set synchronous mode.
      server->bind("set_synchronous_mode", [=](const bool mode) {
        tm->SetSynchronousMode(mode);
      });

      /// Method to set tick timeout for synchronous execution.
      server->bind("set_synchronous_mode_timeout_in_milisecond", [=](const double time) {
        tm->SetSynchronousModeTimeOutInMiliSecond(time);
      });

      /// Method to set randomization seed.
      server->bind("set_random_device_seed", [=](const uint64_t seed) {
        tm->SetRandomDeviceSeed(seed);
      });

      /// Method to provide synchronous tick.
      server->bind("synchronous_tick", [=]() -> bool {
        return tm->SynchronousTick();
      });

      /// Method to check server is alive or not.
      server->bind("health_check_remote_TM", [=](){});

      /// Run traffic manager server to respond of any
      /// user client in asynchronous mode.
      server->async_run();
    }

  }

  ~TrafficManagerServer() {
    if(server) {
      server->stop();
      delete server;
      server = nullptr;
    }
  }

  uint16_t port() const {
    return _RPCPort;
  }

private:

  /// Traffic manager server RPC port
  uint16_t _RPCPort;

  /// Server instance
  ::rpc::server *server = nullptr;

};

} // namespace traffic_manager
} // namespace carla
