// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <thread>

#include "carla/client/detail/Simulator.h"

#include "carla/trafficmanager/TrafficManagerRemote.h"

namespace carla {
namespace traffic_manager {

TrafficManagerRemote::TrafficManagerRemote(
    const std::pair<std::string, uint16_t> &_serverTM,
    carla::client::detail::EpisodeProxy &episodeProxy)
  : client(_serverTM.first, _serverTM.second),
    episodeProxyTM(episodeProxy) {

  Start();

}

/// Destructor.
TrafficManagerRemote::~TrafficManagerRemote() {
  Release();
}

void TrafficManagerRemote::Start() {
  _keep_alive = true;

  std::thread _thread = std::thread([this] () {
    std::chrono::milliseconds wait_time(TM_TIMEOUT);
    try {
      do {
        std::this_thread::sleep_for(wait_time);

        client.HealthCheckRemoteTM();

        /// Until connection active
      } while (_keep_alive);
    } catch (...) {

      std::string rhost("");
      uint16_t rport = 0;

      client.getServerDetails(rhost, rport);

      std::string strtmserver(rhost + ":" + std::to_string(rport));

      /// Create error msg
      std::string errmsg(
          "Trying to connect rpc server of traffic manager; "
          "but the system failed to connect at " + strtmserver);

      /// TSet the error message
      if(_keep_alive) {
        this->episodeProxyTM.Lock()->AddPendingException(errmsg);
      }
    }
    _keep_alive = false;
    _cv.notify_one();
  });

  _thread.detach();
}

void TrafficManagerRemote::Stop() {
  if(_keep_alive) {
    _keep_alive = false;
    std::unique_lock<std::mutex> lock(_mutex);
    std::chrono::milliseconds wait_time(TM_TIMEOUT + 1000);
    _cv.wait_for(lock, wait_time);
  }
}

void TrafficManagerRemote::Release() {
  Stop();
}

void TrafficManagerRemote::Reset() {
  Stop();

  carla::client::detail::EpisodeProxy episode_proxy = episodeProxyTM.Lock()->GetCurrentEpisode();
  episodeProxyTM = episode_proxy;

  Start();
}

void TrafficManagerRemote::RegisterVehicles(const std::vector<ActorPtr> &_actor_list) {
  std::vector<carla::rpc::Actor> actor_list;
  for (auto &&actor : _actor_list) {
    actor_list.emplace_back(actor->Serialize());
  }
  client.RegisterVehicle(actor_list);
}

void TrafficManagerRemote::UnregisterVehicles(const std::vector<ActorPtr> &_actor_list) {
  std::vector<carla::rpc::Actor> actor_list;
  for (auto &&actor : _actor_list) {
    actor_list.emplace_back(actor->Serialize());
  }
  client.UnregisterVehicle(actor_list);
}

void TrafficManagerRemote::SetPercentageSpeedDifference(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetPercentageSpeedDifference(actor, percentage);
}

void TrafficManagerRemote::SetDesiredSpeed(const ActorPtr &_actor, const float value) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetDesiredSpeed(actor, value);
}

void TrafficManagerRemote::SetGlobalPercentageSpeedDifference(const float percentage) {
  client.SetGlobalPercentageSpeedDifference(percentage);
}

void TrafficManagerRemote::SetUpdateVehicleLights(const ActorPtr &_actor, const bool do_update) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetUpdateVehicleLights(actor, do_update);
}

void TrafficManagerRemote::SetCollisionDetection(const ActorPtr &_reference_actor, const ActorPtr &_other_actor, const bool detect_collision) {
  carla::rpc::Actor reference_actor(_reference_actor->Serialize());
  carla::rpc::Actor other_actor(_other_actor->Serialize());

  client.SetCollisionDetection(reference_actor, other_actor, detect_collision);
}

void TrafficManagerRemote::SetForceLaneChange(const ActorPtr &_actor, const bool direction) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetForceLaneChange(actor, direction);
}

void TrafficManagerRemote::SetAutoLaneChange(const ActorPtr &_actor, const bool enable) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetAutoLaneChange(actor, enable);
}

void TrafficManagerRemote::SetDistanceToLeadingVehicle(const ActorPtr &_actor, const float distance) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetDistanceToLeadingVehicle(actor, distance);
}

void TrafficManagerRemote::SetGlobalDistanceToLeadingVehicle(const float distance) {
  client.SetGlobalDistanceToLeadingVehicle(distance);
}


void TrafficManagerRemote::SetPercentageIgnoreWalkers(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetPercentageIgnoreWalkers(actor, percentage);
}

void TrafficManagerRemote::SetPercentageIgnoreVehicles(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetPercentageIgnoreVehicles(actor, percentage);
}

void TrafficManagerRemote::SetPercentageRunningLight(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetPercentageRunningLight(actor, percentage);
}

void TrafficManagerRemote::SetPercentageRunningSign(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetPercentageRunningSign(actor, percentage);
}

void TrafficManagerRemote::SetKeepRightPercentage(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetKeepRightPercentage(actor, percentage);
}

void TrafficManagerRemote::SetRandomLeftLaneChangePercentage(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetRandomLeftLaneChangePercentage(actor, percentage);
}

void TrafficManagerRemote::SetRandomRightLaneChangePercentage(const ActorPtr &_actor, const float percentage) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetRandomRightLaneChangePercentage(actor, percentage);
}

void TrafficManagerRemote::SetHybridPhysicsMode(const bool mode_switch) {
  client.SetHybridPhysicsMode(mode_switch);
}

void TrafficManagerRemote::SetHybridPhysicsRadius(const float radius) {
  client.SetHybridPhysicsRadius(radius);
}

void TrafficManagerRemote::SetOSMMode(const bool mode_switch) {
  client.SetOSMMode(mode_switch);
}

void TrafficManagerRemote::SetCustomPath(const ActorPtr &_actor, const Path path, const bool empty_buffer) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetCustomPath(actor, path, empty_buffer);
}

void TrafficManagerRemote::RemoveUploadPath(const ActorId &actor_id, const bool remove_path) {
  client.RemoveUploadPath(actor_id, remove_path);
}

void TrafficManagerRemote::UpdateUploadPath(const ActorId &actor_id, const Path path) {
  client.UpdateUploadPath(actor_id, path);
}

void TrafficManagerRemote::SetImportedRoute(const ActorPtr &_actor, const Route route, const bool empty_buffer) {
  carla::rpc::Actor actor(_actor->Serialize());

  client.SetImportedRoute(actor, route, empty_buffer);
}

void TrafficManagerRemote::RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) {
  client.RemoveImportedRoute(actor_id, remove_path);
}

void TrafficManagerRemote::UpdateImportedRoute(const ActorId &actor_id, const Route route) {
  client.UpdateImportedRoute(actor_id, route);
}

void TrafficManagerRemote::SetRespawnDormantVehicles(const bool mode_switch) {
  client.SetRespawnDormantVehicles(mode_switch);
}

void TrafficManagerRemote::SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) {
  client.SetBoundariesRespawnDormantVehicles(lower_bound, upper_bound);
}

void TrafficManagerRemote::SetMaxBoundaries(const float lower, const float upper) {
  client.SetMaxBoundaries(lower, upper);
}

void TrafficManagerRemote::ShutDown() {
  client.ShutDown();
}

void TrafficManagerRemote::SetSynchronousMode(bool mode) {
  client.SetSynchronousMode(mode);
}

void TrafficManagerRemote::SetSynchronousModeTimeOutInMiliSecond(double time) {
  client.SetSynchronousModeTimeOutInMiliSecond(time);
}

Action TrafficManagerRemote::GetNextAction(const ActorId &actor_id) {
  return client.GetNextAction(actor_id);
}

ActionBuffer TrafficManagerRemote::GetActionBuffer(const ActorId &actor_id) {
  return client.GetActionBuffer(actor_id);
}

bool TrafficManagerRemote::SynchronousTick() {
  return false;
}

void TrafficManagerRemote::HealthCheckRemoteTM() {
  client.HealthCheckRemoteTM();
}

carla::client::detail::EpisodeProxy& TrafficManagerRemote::GetEpisodeProxy() {
  return episodeProxyTM;
}

void TrafficManagerRemote::SetRandomDeviceSeed(const uint64_t seed) {
  client.SetRandomDeviceSeed(seed);
}

} // namespace traffic_manager
} // namespace carla
