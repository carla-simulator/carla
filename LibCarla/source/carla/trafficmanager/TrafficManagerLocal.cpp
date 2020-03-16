// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/TrafficManagerLocal.h"

#include "carla/client/TrafficLight.h"
#include "carla/client/ActorList.h"
#include "carla/client/DebugHelper.h"

namespace carla {
namespace traffic_manager {

TrafficManagerLocal::TrafficManagerLocal(
    std::vector<float> longitudinal_PID_parameters,
    std::vector<float> longitudinal_highway_PID_parameters,
    std::vector<float> lateral_PID_parameters,
    std::vector<float> lateral_highway_PID_parameters,
    float perc_difference_from_limit,
    carla::client::detail::EpisodeProxy& episodeProxy,
    uint16_t& RPCportTM)
  : longitudinal_PID_parameters(longitudinal_PID_parameters),
    longitudinal_highway_PID_parameters(longitudinal_highway_PID_parameters),
    lateral_PID_parameters(lateral_PID_parameters),
    lateral_highway_PID_parameters(lateral_highway_PID_parameters),
    episodeProxyTM(episodeProxy),
    debug_helper(carla::client::DebugHelper{episodeProxyTM}),
    server(TrafficManagerServer(RPCportTM, static_cast<carla::traffic_manager::TrafficManagerBase *>(this))) {

  parameters.SetGlobalPercentageSpeedDifference(perc_difference_from_limit);

  Start();

}

TrafficManagerLocal::~TrafficManagerLocal() {
  episodeProxyTM.Lock()->DestroyTrafficManager(server.port());
  Release();
}

void TrafficManagerLocal::Start() {

  const carla::SharedPtr<cc::Map> world_map = episodeProxyTM.Lock()->GetCurrentMap();
  local_map = std::make_shared<traffic_manager::InMemoryMap>(world_map);
  local_map->SetUp();

  localization_collision_messenger = std::make_shared<LocalizationToCollisionMessenger>();
  localization_traffic_light_messenger = std::make_shared<LocalizationToTrafficLightMessenger>();
  collision_planner_messenger = std::make_shared<CollisionToPlannerMessenger>();
  localization_planner_messenger = std::make_shared<LocalizationToPlannerMessenger>();
  traffic_light_planner_messenger = std::make_shared<TrafficLightToPlannerMessenger>();
  planner_control_messenger = std::make_shared<PlannerToControlMessenger>();

  localization_stage = std::make_unique<LocalizationStage>(
    "Localization stage",
    localization_planner_messenger, localization_collision_messenger,
    localization_traffic_light_messenger,
    registered_actors, *local_map.get(),
    parameters, debug_helper,
    episodeProxyTM);

  collision_stage = std::make_unique<CollisionStage>(
    "Collision stage",
    localization_collision_messenger, collision_planner_messenger,
    parameters, debug_helper);

  traffic_light_stage = std::make_unique<TrafficLightStage>(
    "Traffic light stage",
    localization_traffic_light_messenger, traffic_light_planner_messenger,
    parameters, debug_helper);

  planner_stage = std::make_unique<MotionPlannerStage>(
    "Motion planner stage",
    localization_planner_messenger,
    collision_planner_messenger,
    traffic_light_planner_messenger,
    planner_control_messenger,
    parameters,
    longitudinal_PID_parameters,
    longitudinal_highway_PID_parameters,
    lateral_PID_parameters,
    lateral_highway_PID_parameters,
    debug_helper);

  control_stage = std::make_unique<BatchControlStage>(
    "Batch control stage",
    planner_control_messenger,
    episodeProxyTM,
    parameters);

  localization_collision_messenger->Start();
  localization_traffic_light_messenger->Start();
  localization_planner_messenger->Start();
  collision_planner_messenger->Start();
  traffic_light_planner_messenger->Start();
  planner_control_messenger->Start();

  localization_stage->Start();
  collision_stage->Start();
  traffic_light_stage->Start();
  planner_stage->Start();
  control_stage->Start();

}

void TrafficManagerLocal::Stop() {
  localization_collision_messenger->Stop();
  localization_traffic_light_messenger->Stop();
  localization_planner_messenger->Stop();
  collision_planner_messenger->Stop();
  traffic_light_planner_messenger->Stop();
  planner_control_messenger->Stop();

  localization_stage->Stop();
  collision_stage->Stop();
  traffic_light_stage->Stop();
  planner_stage->Stop();
  control_stage->Stop();
}

void TrafficManagerLocal::Release() {
  Stop();
  localization_collision_messenger.reset();
  localization_traffic_light_messenger.reset();
  localization_planner_messenger.reset();
  collision_planner_messenger.reset();
  traffic_light_planner_messenger.reset();
  planner_control_messenger.reset();
  localization_stage.reset();
  collision_stage.reset();
  traffic_light_stage.reset();
  planner_stage.reset();
  control_stage.reset();
}

void TrafficManagerLocal::Reset() {

  Release();

  carla::client::detail::EpisodeProxy episode_proxy = episodeProxyTM.Lock()->GetCurrentEpisode();
  episodeProxyTM = episode_proxy;

  Start();
}

void TrafficManagerLocal::RegisterVehicles(const std::vector<ActorPtr>& actor_list) {
  registered_actors.Insert(actor_list);
}

void TrafficManagerLocal::UnregisterVehicles(const std::vector<ActorPtr>& actor_list) {
  registered_actors.Remove(actor_list);
}

void TrafficManagerLocal::SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) {
  parameters.SetPercentageSpeedDifference(actor, percentage);
}

void TrafficManagerLocal::SetGlobalPercentageSpeedDifference(const float percentage) {
  parameters.SetGlobalPercentageSpeedDifference(percentage);
}

void TrafficManagerLocal::SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) {
  parameters.SetCollisionDetection(reference_actor, other_actor, detect_collision);
}

void TrafficManagerLocal::SetForceLaneChange(const ActorPtr &actor, const bool direction) {
  parameters.SetForceLaneChange(actor, direction);
}

void TrafficManagerLocal::SetAutoLaneChange(const ActorPtr &actor, const bool enable) {
  parameters.SetAutoLaneChange(actor, enable);
}

void TrafficManagerLocal::SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {
  parameters.SetDistanceToLeadingVehicle(actor, distance);
}

void TrafficManagerLocal::SetGlobalDistanceToLeadingVehicle(const float distance) {
  parameters.SetGlobalDistanceToLeadingVehicle(distance);
}

void TrafficManagerLocal::SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) {
  parameters.SetPercentageIgnoreWalkers(actor, perc);
}

void TrafficManagerLocal::SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) {
  parameters.SetPercentageIgnoreVehicles(actor, perc);
}

void TrafficManagerLocal::SetPercentageRunningLight(const ActorPtr &actor, const float perc) {
  parameters.SetPercentageRunningLight(actor, perc);
}

void TrafficManagerLocal::SetPercentageRunningSign(const ActorPtr &actor, const float perc) {
  parameters.SetPercentageRunningSign(actor, perc);
}

void TrafficManagerLocal::SetKeepRightPercentage(const ActorPtr &actor, const float percentage) {
  parameters.SetKeepRightPercentage(actor, percentage);
}

bool TrafficManagerLocal::CheckAllFrozen(TLGroup tl_to_freeze) {
  for (auto& elem : tl_to_freeze) {
    if (!elem->IsFrozen() || elem->GetState() != TLS::Red) {
      return false;
    }
  }
  return true;
}

void TrafficManagerLocal::ResetAllTrafficLights() {

  auto Filter = [&](auto &actors, auto &wildcard_pattern) {
    std::vector<carla::client::detail::ActorVariant> filtered;
    for (auto &&actor : actors) {
      if (carla::StringUtil::Match(carla::client::detail::ActorVariant(actor).GetTypeId(), wildcard_pattern)) {
        filtered.push_back(actor);
      }
    }
    return filtered;
  };

  // Get all actors of the world.
  auto world_actorsList = episodeProxyTM.Lock()->GetAllTheActorsInTheEpisode();

  // Filter based on wildcard pattern.
  const auto world_traffic_lights = Filter(world_actorsList, "*traffic_light*");

  std::vector<TLGroup> list_of_all_groups;
  TLGroup tl_to_freeze;
  std::vector<carla::ActorId> list_of_ids;
  for (auto tl : world_traffic_lights) {
    if (!(std::find(list_of_ids.begin(), list_of_ids.end(), tl.GetId()) != list_of_ids.end())) {
      const TLGroup tl_group = boost::static_pointer_cast<cc::TrafficLight>(tl.Get(episodeProxyTM))->GetGroupTrafficLights();
      list_of_all_groups.push_back(tl_group);
      for (uint64_t i=0u; i<tl_group.size(); i++) {
        list_of_ids.push_back(tl_group.at(i).get()->GetId());
        if(i!=0u) {
          tl_to_freeze.push_back(tl_group.at(i));
        }
      }
    }
  }

  for (TLGroup& tl_group : list_of_all_groups) {
    tl_group.front()->SetState(TLS::Green);
    std::for_each(
        tl_group.begin()+1, tl_group.end(),
        [] (auto& tl) {tl->SetState(TLS::Red);});
  }

  while (!CheckAllFrozen(tl_to_freeze)) {
    for (auto& tln : tl_to_freeze) {
      tln->SetState(TLS::Red);
      tln->Freeze(true);
    }
  }

}

void TrafficManagerLocal::SetSynchronousMode(bool mode) {
  parameters.SetSynchronousMode(mode);
}

void TrafficManagerLocal::SetSynchronousModeTimeOutInMiliSecond(double time) {
  parameters.SetSynchronousModeTimeOutInMiliSecond(time);
}

bool TrafficManagerLocal::SynchronousTick() {
  return control_stage->RunStep();
}

carla::client::detail::EpisodeProxy& TrafficManagerLocal::GetEpisodeProxy() {
  return episodeProxyTM;
}

std::vector<ActorId> TrafficManagerLocal::GetRegisteredVehiclesIDs() {

  return registered_actors.GetIDList();
}

} // namespace traffic_manager
} // namespace carla
