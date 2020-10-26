// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

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

void TrafficManagerRemote::SetGlobalPercentageSpeedDifference(const float percentage) {
  client.SetGlobalPercentageSpeedDifference(percentage);
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

void TrafficManagerRemote::SetHybridPhysicsMode(const bool mode_switch) {
  client.SetHybridPhysicsMode(mode_switch);
}

void TrafficManagerRemote::SetHybridPhysicsRadius(const float radius) {
  client.SetHybridPhysicsRadius(radius);
}

void TrafficManagerRemote::SetOSMMode(const bool mode_switch) {
  client.SetOSMMode(mode_switch);
}

void TrafficManagerRemote::SetSynchronousMode(bool mode) {
  client.SetSynchronousMode(mode);
}

void TrafficManagerRemote::SetSynchronousModeTimeOutInMiliSecond(double time) {
  client.SetSynchronousModeTimeOutInMiliSecond(time);
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
