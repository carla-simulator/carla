// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/TrafficManagerLocal.h"

#include "carla/trafficmanager/ALSM.h"
#include "carla/trafficmanager/CollisionAvoidance.h"
#include "carla/trafficmanager/Localization.h"
#include "carla/trafficmanager/MotionPlan.h"
#include "carla/trafficmanager/TrafficLightResponse.h"

namespace carla {
namespace traffic_manager {

using namespace constants::FrameMemory;

TrafficManagerLocal::TrafficManagerLocal(
    std::vector<float> longitudinal_PID_parameters,
    std::vector<float> longitudinal_highway_PID_parameters,
    std::vector<float> lateral_PID_parameters,
    std::vector<float> lateral_highway_PID_parameters,
    float perc_difference_from_limit,
    cc::detail::EpisodeProxy& episode_proxy,
    uint16_t& RPCportTM)
  : episode_proxy(episode_proxy),
    world(cc::World(episode_proxy)),
    longitudinal_PID_parameters(longitudinal_PID_parameters),
    longitudinal_highway_PID_parameters(longitudinal_highway_PID_parameters),
    lateral_PID_parameters(lateral_PID_parameters),
    lateral_highway_PID_parameters(lateral_highway_PID_parameters),
    debug_helper(world.MakeDebugHelper()),
    server(TrafficManagerServer(RPCportTM, static_cast<carla::traffic_manager::TrafficManagerBase *>(this))) {

  parameters.SetGlobalPercentageSpeedDifference(perc_difference_from_limit);

  registered_vehicles_state = -1;

  buffer_map = std::make_shared<BufferMap>();

  collision_frame_ptr = std::make_shared<CollisionFrame>(INITIAL_SIZE);
  tl_frame_ptr = std::make_shared<TLFrame>(INITIAL_SIZE);
  control_frame_ptr = std::make_shared<ControlFrame>(INITIAL_SIZE);

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

  while (run_traffic_manger.load()) {

    bool synchronous_mode = parameters.GetSynchronousMode();
    bool hybrid_physics_mode = parameters.GetHybridPhysicsMode();

    // Wait for external trigger to initiate cycle in synchronous mode.
    if (synchronous_mode) {
      std::unique_lock<std::mutex> lock(step_execution_mutex);
      while (!step_begin.load()) {
        step_begin_trigger.wait(lock, [this]() {return step_begin.load();});
      }
      step_begin.store(false);
    }

    // Skipping velocity update if elapsed time is less than 0.05s in asynchronous, hybrid mode.
    if (!synchronous_mode && hybrid_physics_mode)
    {
      TimePoint current_instance = chr::system_clock::now();
      chr::duration<float> elapsed_time = current_instance - previous_update_instance;
      // TODO: move all constants to one file.
      if (elapsed_time.count() > 0.05) // HYBRID_MODE_DT
      {
        previous_update_instance = current_instance;
      } else {
        continue;
      }
    }

    // snippet_profiler.MeasureExecutionTime("ALSM", true);
    // TODO: Perform cleanup for traffic light response related strucutres too.
    AgentLifecycleAndStateManagement(registered_vehicles,
                                     vehicle_id_list,
                                     unregistered_actors,
                                     registered_vehicles_state,
                                     buffer_map,
                                     track_traffic,
                                     idle_time,
                                     hero_actors,
                                     last_lane_change_location,
                                     kinematic_state_map,
                                     static_attribute_map,
                                     tl_state_map,
                                     elapsed_last_actor_destruction,
                                     parameters,
                                     world,
                                     local_map);
    // snippet_profiler.MeasureExecutionTime("ALSM", false);

    int current_registered_vehicles_state = registered_vehicles.GetState();
    unsigned long number_of_vehicles = vehicle_id_list.size();
    if (registered_vehicles_state != current_registered_vehicles_state
        || number_of_vehicles != registered_vehicles.Size()){

      vehicle_id_list = registered_vehicles.GetIDList();
      number_of_vehicles = vehicle_id_list.size();
      unsigned long new_frame_size = INITIAL_SIZE + GROWTH_STEP_SIZE * (number_of_vehicles / GROWTH_STEP_SIZE);
      if (new_frame_size != control_frame_ptr->size())
      {
        collision_frame_ptr = std::make_shared<CollisionFrame>(new_frame_size);
        tl_frame_ptr = std::make_shared<TLFrame>(new_frame_size);
        control_frame_ptr = std::make_shared<ControlFrame>(new_frame_size);
      }
      registered_vehicles_state = registered_vehicles.GetState();
    }

    // snippet_profiler.MeasureExecutionTime("Localization", true);
    for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index)
    {
      Localization(index,
                   vehicle_id_list,
                   buffer_map,
                   kinematic_state_map,
                   track_traffic,
                   local_map,
                   parameters,
                   last_lane_change_location);
    }
    // snippet_profiler.MeasureExecutionTime("Localization", false);

    // snippet_profiler.MeasureExecutionTime("Collision", true);
    for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index)
    {
      CollisionAvoidance(index,
                         vehicle_id_list,
                         kinematic_state_map,
                         static_attribute_map,
                         tl_state_map,
                         buffer_map,
                         track_traffic,
                         parameters,
                         collision_locks,
                         collision_frame_ptr);
    }
    // snippet_profiler.MeasureExecutionTime("Collision", false);

    // snippet_profiler.MeasureExecutionTime("TrafficLight", true);
    for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index)
    {
      TrafficLightResponse(index,
                           vehicle_id_list,
                           tl_state_map,
                           buffer_map,
                           parameters,
                           vehicle_last_ticket,
                           junction_last_ticket,
                           vehicle_last_junction,
                           tl_frame_ptr);
    }
    // snippet_profiler.MeasureExecutionTime("TrafficLight", false);

    // snippet_profiler.MeasureExecutionTime("MotionPlan", true);
    for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index)
    {
      MotionPlan(index,
                 vehicle_id_list,
                 kinematic_state_map,
                 static_attribute_map,
                 parameters,
                 buffer_map,
                 longitudinal_PID_parameters,
                 longitudinal_highway_PID_parameters,
                 lateral_PID_parameters,
                 lateral_highway_PID_parameters,
                 collision_frame_ptr,
                 tl_frame_ptr,
                 pid_state_map,
                 teleportation_instance,
                 control_frame_ptr);
    }
    // snippet_profiler.MeasureExecutionTime("MotionPlan", false);

    std::vector<carla::rpc::Command> batch_command(number_of_vehicles);
    for (unsigned long i = 0u; i < number_of_vehicles; ++i) {
      batch_command.at(i) = control_frame_ptr->at(i);
    }

    if (synchronous_mode)
    {
      episode_proxy.Lock()->ApplyBatchSync(std::move(batch_command), false);

      step_end.store(true);
      step_end_trigger.notify_one();
    }
    else
    {
      episode_proxy.Lock()->ApplyBatch(std::move(batch_command), false);
    }
  }
}

bool TrafficManagerLocal::SynchronousTick() {
  if (parameters.GetSynchronousMode()) {
    step_begin.store(true);
    step_begin_trigger.notify_one();

    std::unique_lock<std::mutex> lock(step_execution_mutex);
    while (!step_end.load()) {
      step_end_trigger.wait(lock, [this]() {return step_end.load();});
    }
    step_end.store(false);
  }
  return true;
}

void TrafficManagerLocal::Stop() {

  run_traffic_manger.store(false);

  if(worker_thread) {
    if(worker_thread->joinable()){
      worker_thread->join();
    }
    worker_thread.release();
  }

  vehicle_id_list.clear();
  registered_vehicles.Clear();
  registered_vehicles_state = -1;
  unregistered_actors.clear();
  track_traffic.Clear();
  idle_time.clear();
  elapsed_last_actor_destruction = 0.0f;
  hero_actors.clear();
  kinematic_state_map.clear();
  static_attribute_map.clear();
  tl_state_map.clear();
  previous_update_instance = chr::system_clock::now();
  last_lane_change_location.clear();
}

void TrafficManagerLocal::Release() {

  Stop();

  buffer_map.reset();
  local_map.reset();
  collision_frame_ptr.reset();
  tl_frame_ptr.reset();
  control_frame_ptr.reset();
}

void TrafficManagerLocal::Reset() {

  Release();
  episode_proxy = episode_proxy.Lock()->GetCurrentEpisode();
  world = cc::World(episode_proxy);
  SetupLocalMap();
  Start();
}

void TrafficManagerLocal::RegisterVehicles(const std::vector<ActorPtr>& vehicle_list) {
  registered_vehicles.Insert(vehicle_list);
}

void TrafficManagerLocal::UnregisterVehicles(const std::vector<ActorPtr>& actor_list) {
  std::vector<ActorId> actor_id_list;
  for (auto &actor: actor_list) {
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
  for (auto& elem : tl_to_freeze) {
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

carla::client::detail::EpisodeProxy& TrafficManagerLocal::GetEpisodeProxy() {
  return episode_proxy;
}

std::vector<ActorId> TrafficManagerLocal::GetRegisteredVehiclesIDs() {
  return registered_vehicles.GetIDList();
}

} // namespace traffic_manager
} // namespace carla
