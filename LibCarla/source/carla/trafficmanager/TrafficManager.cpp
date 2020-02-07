// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficManager.h"

#include "carla/client/TrafficLight.h"

namespace carla {
namespace traffic_manager {

  TrafficManager::TrafficManager(
      std::vector<float> longitudinal_PID_parameters,
      std::vector<float> longitudinal_highway_PID_parameters,
      std::vector<float> lateral_PID_parameters,
      std::vector<float> lateral_highway_PID_parameters,
      float perc_difference_from_limit,
      cc::Client &client_connection)
    : longitudinal_PID_parameters(longitudinal_PID_parameters),
      longitudinal_highway_PID_parameters(longitudinal_highway_PID_parameters),
      lateral_PID_parameters(lateral_PID_parameters),
      lateral_highway_PID_parameters(lateral_highway_PID_parameters),
      client_connection(client_connection),
      world(client_connection.GetWorld()),
      debug_helper(client_connection.GetWorld().MakeDebugHelper()) {

    const WorldMap world_map = world.GetMap();
    local_map = std::make_shared<traffic_manager::InMemoryMap>(world_map);
    local_map->SetUp();

    parameters.SetGlobalPercentageSpeedDifference(perc_difference_from_limit);

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
        world);

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
        planner_control_messenger, client_connection);

    Start();
  }

  TrafficManager::~TrafficManager() {

    Stop();
  }

  std::unique_ptr<TrafficManager> TrafficManager::singleton_pointer = nullptr;

  TrafficManager& TrafficManager::GetInstance(cc::Client &client_connection) {

    if (singleton_pointer == nullptr) {

      const std::vector<float> longitudinal_param = {2.0f, 0.05f, 0.07f};
      const std::vector<float> longitudinal_highway_param = {4.0f, 0.02f, 0.03f};
      const std::vector<float> lateral_param = {10.0f, 0.02f, 1.0f};
      const std::vector<float> lateral_highway_param = {9.0f, 0.02f, 1.0f};
      const float perc_difference_from_limit = 30.0f;

      TrafficManager* tm_ptr = new TrafficManager(
        longitudinal_param, longitudinal_highway_param, lateral_param, lateral_highway_param,
        perc_difference_from_limit, client_connection
      );

      singleton_pointer = std::unique_ptr<TrafficManager>(tm_ptr);
    }

    return *singleton_pointer.get();
  }

  std::unique_ptr<cc::Client> TrafficManager::singleton_local_client = nullptr;

  cc::Client& TrafficManager::GetUniqueLocalClient() {

    if (singleton_local_client == nullptr) {
      cc::Client* client = new cc::Client("localhost", 2000);
      singleton_local_client = std::unique_ptr<cc::Client>(client);
    }

    return *singleton_local_client.get();
  }

  void TrafficManager::RegisterVehicles(const std::vector<ActorPtr> &actor_list) {
    registered_actors.Insert(actor_list);
  }

  void TrafficManager::UnregisterVehicles(const std::vector<ActorPtr> &actor_list) {
    registered_actors.Remove(actor_list);
  }

  void TrafficManager::Start() {

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

  void TrafficManager::Stop() {

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

  void TrafficManager::SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) {
    parameters.SetPercentageSpeedDifference(actor, percentage);
  }

  void TrafficManager::SetGlobalPercentageSpeedDifference(const float percentage) {
    parameters.SetGlobalPercentageSpeedDifference(percentage);
  }

  void TrafficManager::SetCollisionDetection(
      const ActorPtr &reference_actor,
      const ActorPtr &other_actor,
      const bool detect_collision) {

    parameters.SetCollisionDetection(reference_actor, other_actor, detect_collision);
  }

  void TrafficManager::SetForceLaneChange(const ActorPtr &actor, const bool direction) {

    parameters.SetForceLaneChange(actor, direction);
  }

  void TrafficManager::SetAutoLaneChange(const ActorPtr &actor, const bool enable) {

    parameters.SetAutoLaneChange(actor, enable);
  }

  void TrafficManager::SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {

    parameters.SetDistanceToLeadingVehicle(actor, distance);
  }

  void TrafficManager::SetPercentageIgnoreActors(const ActorPtr &actor, const float perc) {

    parameters.SetPercentageIgnoreActors(actor, perc);
  }

  void TrafficManager::SetPercentageRunningLight(const ActorPtr &actor, const float perc) {

    parameters.SetPercentageRunningLight(actor, perc);
  }


  bool TrafficManager::CheckAllFrozen(TLGroup tl_to_freeze) {
    for (auto& elem : tl_to_freeze) {
      if (!elem->IsFrozen() || elem->GetState() != TLS::Red) {
        return false;
      }
    }
    return true;
  }

  void TrafficManager::ResetAllTrafficLights() {
    const auto world_traffic_lights = world.GetActors()->Filter("*traffic_light*");

    std::vector<TLGroup> list_of_all_groups;
    TLGroup tl_to_freeze;
    std::vector<carla::ActorId> list_of_ids;
    for (auto tl : *world_traffic_lights.get()) {
      if (!(std::find(list_of_ids.begin(), list_of_ids.end(), tl->GetId()) != list_of_ids.end())) {
        const TLGroup tl_group = boost::static_pointer_cast<cc::TrafficLight>(tl)->GetGroupTrafficLights();
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

} // namespace traffic_manager
} // namespace carla
