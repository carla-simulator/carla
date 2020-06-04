// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/Simulator.h"

#include "carla/trafficmanager/TrafficManagerLocal.h"

namespace carla {
namespace traffic_manager {

using namespace constants::FrameMemory;

TrafficManagerLocal::TrafficManagerLocal(
  std::vector<float> longitudinal_PID_parameters,
  std::vector<float> longitudinal_highway_PID_parameters,
  std::vector<float> lateral_PID_parameters,
  std::vector<float> lateral_highway_PID_parameters,
  float perc_difference_from_limit,
  cc::detail::EpisodeProxy &episode_proxy,
  uint16_t &RPCportTM)

  : longitudinal_PID_parameters(longitudinal_PID_parameters),
    longitudinal_highway_PID_parameters(longitudinal_highway_PID_parameters),
    lateral_PID_parameters(lateral_PID_parameters),
    lateral_highway_PID_parameters(lateral_highway_PID_parameters),

    episode_proxy(episode_proxy),
    world(cc::World(episode_proxy)),
    debug_helper(world.MakeDebugHelper()),

    localization_stage(LocalizationStage(vehicle_id_list,
                                         buffer_map,
                                         simulation_state,
                                         track_traffic,
                                         local_map,
                                         parameters,
                                         localization_frame,
                                         debug_helper)),

    collision_stage(CollisionStage(vehicle_id_list,
                                   simulation_state,
                                   buffer_map,
                                   track_traffic,
                                   parameters,
                                   collision_frame,
                                   debug_helper)),

    traffic_light_stage(TrafficLightStage(vehicle_id_list,
                                          simulation_state,
                                          buffer_map,
                                          parameters,
                                          tl_frame)),

    motion_plan_stage(MotionPlanStage(vehicle_id_list,
                                      simulation_state,
                                      parameters,
                                      buffer_map,
                                      track_traffic,
                                      longitudinal_PID_parameters,
                                      longitudinal_highway_PID_parameters,
                                      lateral_PID_parameters,
                                      lateral_highway_PID_parameters,
                                      localization_frame,
                                      collision_frame,
                                      tl_frame,
                                      control_frame)),

    alsm(ALSM(registered_vehicles,
              buffer_map,
              track_traffic,
              parameters,
              world,
              local_map,
              simulation_state,
              localization_stage,
              collision_stage,
              traffic_light_stage,
              motion_plan_stage)),

    server(TrafficManagerServer(RPCportTM, static_cast<carla::traffic_manager::TrafficManagerBase *>(this))) {

  parameters.SetGlobalPercentageSpeedDifference(perc_difference_from_limit);

  registered_vehicles_state = -1;

  SetupLocalMap();

  Start();
}

TrafficManagerLocal::~TrafficManagerLocal() {
  episode_proxy.Lock()->DestroyTrafficManager(server.port());
  Release();
}

void TrafficManagerLocal::SetupLocalMap() {
  const carla::SharedPtr<cc::Map> world_map = world.GetMap();
  local_map = std::make_shared<InMemoryMap>(world_map);
  local_map->SetUp();
}

void TrafficManagerLocal::Start() {
  run_traffic_manger.store(true);
  worker_thread = std::make_unique<std::thread>(&TrafficManagerLocal::Run, this);
}

void TrafficManagerLocal::Run() {

  localization_frame.reserve(INITIAL_SIZE);
  collision_frame.reserve(INITIAL_SIZE);
  tl_frame.reserve(INITIAL_SIZE);
  control_frame.reserve(INITIAL_SIZE);
  current_reserved_capacity = INITIAL_SIZE;

  while (run_traffic_manger.load()) {
    bool synchronous_mode = parameters.GetSynchronousMode();
    bool hybrid_physics_mode = parameters.GetHybridPhysicsMode();

    // Wait for external trigger to initiate cycle in synchronous mode.
    if (synchronous_mode) {
      std::unique_lock<std::mutex> lock(step_execution_mutex);
      step_begin_trigger.wait(lock, [this]() {return step_begin.load();});
      step_begin.store(false);
    }

    // Skipping velocity update if elapsed time is less than 0.05s in asynchronous, hybrid mode.
    if (!synchronous_mode && hybrid_physics_mode) {
      TimePoint current_instance = chr::system_clock::now();
      chr::duration<float> elapsed_time = current_instance - previous_update_instance;
      float time_to_wait = HYBRID_MODE_DT - elapsed_time.count();
      if (time_to_wait > 0.0f) {
        std::this_thread::sleep_for(chr::duration<float>(time_to_wait));
      }
      previous_update_instance = current_instance;
    }

    // Updating simulation state, actor life cycle and performing necessary cleanup.
    alsm.Update();

    // Re-allocating inter-stage communication frames based on changed number of registered vehicles.
    int current_registered_vehicles_state = registered_vehicles.GetState();
    unsigned long number_of_vehicles = vehicle_id_list.size();
    if (registered_vehicles_state != current_registered_vehicles_state || number_of_vehicles != registered_vehicles.Size()) {

      vehicle_id_list = registered_vehicles.GetIDList();
      number_of_vehicles = vehicle_id_list.size();

      // Reserve more space if needed.
      uint64_t growth_factor = static_cast<uint64_t>(static_cast<float>(number_of_vehicles) * INV_GROWTH_STEP_SIZE);
      uint64_t new_frame_capacity = INITIAL_SIZE + GROWTH_STEP_SIZE * growth_factor;
      if (new_frame_capacity > current_reserved_capacity) {
        localization_frame.reserve(new_frame_capacity);
        collision_frame.reserve(new_frame_capacity);
        tl_frame.reserve(new_frame_capacity);
        control_frame.reserve(new_frame_capacity);
      }

      registered_vehicles_state = registered_vehicles.GetState();
    }

    // Reset frames for current cycle.
    localization_frame.clear();
    localization_frame.resize(number_of_vehicles);
    collision_frame.clear();
    collision_frame.resize(number_of_vehicles);
    tl_frame.clear();
    tl_frame.resize(number_of_vehicles);
    control_frame.clear();
    control_frame.resize(number_of_vehicles);

    // Run core operation stages.
    for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index) {
      localization_stage.Update(index);
    }

    for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index) {
      collision_stage.Update(index);
      traffic_light_stage.Update(index);
      motion_plan_stage.Update(index);
    }
    collision_stage.ClearCycleCache();

    // Building the command array for current cycle.
    std::vector<carla::rpc::Command> batch_command(number_of_vehicles);
    for (unsigned long i = 0u; i < number_of_vehicles; ++i) {
      batch_command.at(i) = control_frame.at(i);
    }

    // Sending the current cycle's batch command to the simulator.
    episode_proxy.Lock()->ApplyBatchSync(std::move(batch_command), false);
    if (synchronous_mode) {
      step_end.store(true);
      step_end_trigger.notify_one();
    }
  }
}

bool TrafficManagerLocal::SynchronousTick() {
  if (parameters.GetSynchronousMode()) {
    step_begin.store(true);
    step_begin_trigger.notify_one();

    std::unique_lock<std::mutex> lock(step_execution_mutex);
    step_end_trigger.wait(lock, [this]() { return step_end.load(); });
    step_end.store(false);
  }
  return true;
}

void TrafficManagerLocal::Stop() {

  run_traffic_manger.store(false);

  if (worker_thread) {
    if (worker_thread->joinable()) {
      worker_thread->join();
    }
    worker_thread.release();
  }

  vehicle_id_list.clear();
  registered_vehicles.Clear();
  registered_vehicles_state = -1;
  track_traffic.Clear();
  previous_update_instance = chr::system_clock::now();
  current_reserved_capacity = 0u;

  simulation_state.Reset();
  localization_stage.Reset();
  collision_stage.Reset();
  traffic_light_stage.Reset();
  motion_plan_stage.Reset();

  buffer_map.clear();
  localization_frame.clear();
  collision_frame.clear();
  tl_frame.clear();
  control_frame.clear();

  run_traffic_manger.store(true);
  step_begin.store(false);
  step_end.store(false);
}

void TrafficManagerLocal::Release() {

  Stop();

  local_map.reset();
}

void TrafficManagerLocal::Reset() {

  Release();
  episode_proxy = episode_proxy.Lock()->GetCurrentEpisode();
  world = cc::World(episode_proxy);
  SetupLocalMap();
  Start();
}

void TrafficManagerLocal::RegisterVehicles(const std::vector<ActorPtr> &vehicle_list) {
  registered_vehicles.Insert(vehicle_list);
}

void TrafficManagerLocal::UnregisterVehicles(const std::vector<ActorPtr> &actor_list) {

  std::vector<ActorId> actor_id_list;
  for (auto &actor : actor_list) {
    actor_id_list.push_back(actor->GetId());
  }
  registered_vehicles.Remove(actor_id_list);
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

void TrafficManagerLocal::SetHybridPhysicsMode(const bool mode_switch) {
  parameters.SetHybridPhysicsMode(mode_switch);
}

void TrafficManagerLocal::SetHybridPhysicsRadius(const float radius) {
  parameters.SetHybridPhysicsRadius(radius);
}

bool TrafficManagerLocal::CheckAllFrozen(TLGroup tl_to_freeze) {
  for (auto &elem : tl_to_freeze) {
    if (!elem->IsFrozen() || elem->GetState() != TLS::Red) {
      return false;
    }
  }
  return true;
}

void TrafficManagerLocal::ResetAllTrafficLights() {

  // Filter based on wildcard pattern.
  const auto world_traffic_lights = world.GetActors()->Filter("*traffic_light*");

  std::vector<TLGroup> list_of_all_groups;
  TLGroup tl_to_freeze;
  std::vector<carla::ActorId> list_of_ids;
  for (auto iter = world_traffic_lights->begin(); iter != world_traffic_lights->end(); iter++) {
    auto tl = *iter;
    if (!(std::find(list_of_ids.begin(), list_of_ids.end(), tl->GetId()) != list_of_ids.end())) {
      const TLGroup tl_group = boost::static_pointer_cast<cc::TrafficLight>(tl)->GetGroupTrafficLights();
      list_of_all_groups.push_back(tl_group);
      for (uint64_t i = 0u; i < tl_group.size(); i++) {
        list_of_ids.push_back(tl_group.at(i).get()->GetId());
        if (i != 0u) {
          tl_to_freeze.push_back(tl_group.at(i));
        }
      }
    }
  }

  for (TLGroup &tl_group : list_of_all_groups) {
    tl_group.front()->SetState(TLS::Green);
    std::for_each(
        tl_group.begin() + 1, tl_group.end(),
        [](auto &tl) { tl->SetState(TLS::Red); });
  }

  while (!CheckAllFrozen(tl_to_freeze)) {
    for (auto &tln : tl_to_freeze) {
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

carla::client::detail::EpisodeProxy &TrafficManagerLocal::GetEpisodeProxy() {
  return episode_proxy;
}

std::vector<ActorId> TrafficManagerLocal::GetRegisteredVehiclesIDs() {
  return registered_vehicles.GetIDList();
}

} // namespace traffic_manager
} // namespace carla
