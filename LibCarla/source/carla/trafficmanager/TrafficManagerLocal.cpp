// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <algorithm>

#include "carla/Logging.h"

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

    localization_stage(LocalizationStage(vehicle_id_list,
                                         buffer_map,
                                         simulation_state,
                                         track_traffic,
                                         local_map,
                                         parameters,
                                         marked_for_removal,
                                         localization_frame,
                                         random_devices)),

    collision_stage(CollisionStage(vehicle_id_list,
                                   simulation_state,
                                   buffer_map,
                                   track_traffic,
                                   parameters,
                                   collision_frame,
                                   random_devices)),

    traffic_light_stage(TrafficLightStage(vehicle_id_list,
                                          simulation_state,
                                          buffer_map,
                                          parameters,
                                          world,
                                          tl_frame,
                                          random_devices)),

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
                                      world,
                                      control_frame,
                                      random_devices,
                                      local_map)),
                              
    vehicle_light_stage(VehicleLightStage(simulation_state,
                                          buffer_map,
                                          world,
                                          control_frame)),

    alsm(ALSM(registered_vehicles,
              buffer_map,
              track_traffic,
              marked_for_removal,
              parameters,
              world,
              local_map,
              simulation_state,
              localization_stage,
              collision_stage,
              traffic_light_stage,
              motion_plan_stage,
              vehicle_light_stage,
              random_devices)),

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
  const carla::SharedPtr<const cc::Map> world_map = world.GetMap();
  local_map = std::make_shared<InMemoryMap>(world_map);

  auto files = episode_proxy.Lock()->GetRequiredFiles("TM");
  if (!files.empty()) {
    auto content = episode_proxy.Lock()->GetCacheFile(files[0], true);
    if (content.size() != 0) {
      local_map->Load(content);
    } else {
      log_warning("No InMemoryMap cache found. Setting up local map. This may take a while...");
      local_map->SetUp();
    }
  } else {
    log_warning("No InMemoryMap cache found. Setting up local map. This may take a while...");
    local_map->SetUp();
  }
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

  size_t last_frame = 0;
  while (run_traffic_manger.load()) {

    bool synchronous_mode = parameters.GetSynchronousMode();
    bool hybrid_physics_mode = parameters.GetHybridPhysicsMode();
    parameters.SetMaxBoundaries(20.0f, episode_proxy.Lock()->GetEpisodeSettings().actor_active_distance);

    // Wait for external trigger to initiate cycle in synchronous mode.
    if (synchronous_mode) {
      std::unique_lock<std::mutex> lock(step_execution_mutex);
      step_begin_trigger.wait(lock, [this]() {return step_begin.load() || !run_traffic_manger.load();});
      step_begin.store(false);
    }

    // Skipping velocity update if elapsed time is less than 0.05s in asynchronous, hybrid mode.
    if (!synchronous_mode && hybrid_physics_mode) {
      TimePoint current_instance = chr::system_clock::now();
      chr::duration<float> elapsed_time = current_instance - previous_update_instance;
      chr::duration<float> time_to_wait = chr::duration<float>(HYBRID_MODE_DT) - elapsed_time;
      if (time_to_wait > chr::duration<float>(0.0f)) {
        std::this_thread::sleep_for(time_to_wait);
      }
      previous_update_instance = current_instance;
    }

    // Stop TM from processing the same frame more than once
    if (!synchronous_mode) {
      carla::client::Timestamp timestamp = world.GetSnapshot().GetTimestamp();
      if (timestamp.frame == last_frame) {
        continue;
      }
      last_frame = timestamp.frame;
    }

    std::unique_lock<std::mutex> registration_lock(registration_mutex);
    // Updating simulation state, actor life cycle and performing necessary cleanup.
    alsm.Update();


    // Re-allocating inter-stage communication frames based on changed number of registered vehicles.
    int current_registered_vehicles_state = registered_vehicles.GetState();
    unsigned long number_of_vehicles = vehicle_id_list.size();
    if (registered_vehicles_state != current_registered_vehicles_state || number_of_vehicles != registered_vehicles.Size()) {

      vehicle_id_list = registered_vehicles.GetIDList();

      std::sort(vehicle_id_list.begin(), vehicle_id_list.end());

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
    ///////////////////////////////////////////////
    control_frame.reserve(2 * number_of_vehicles);
    ///////////////////////////////////////////////
    control_frame.resize(number_of_vehicles);

    // Run core operation stages.

    for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index) {
      localization_stage.Update(index);
    }
    for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index) {
      collision_stage.Update(index);
    }
    collision_stage.ClearCycleCache();
    vehicle_light_stage.ClearCycleCache();
    for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index) {
      traffic_light_stage.Update(index);
      motion_plan_stage.Update(index);
      vehicle_light_stage.Update(index);
    }

    ///////////////////////////////////////////////
    // rpc::VehicleLightStateList all_light_states = world.GetVehiclesLightStates();
    // size_t appliedLightStateCount = 0;
    // for (unsigned long index = 0u; index < all_light_states.size(); ++index) {
    //   ActorId id = all_light_states[index].first;
    //   rpc::VehicleLightState::flag_type light_states = all_light_states[index].second;
    //   bool brake_lights = false;
    //   bool left_turn_indicator = false;
    //   bool right_turn_indicator = false;
    //   bool position = false;
    //   bool low_beam = false;
    //   bool high_beam = false;
    //   bool fog_lights = false;

    //   cg::Vector3D actor_vec = simulation_state.GetHeading(id);

    //   // Recover the planned waypoints for this vehicle
    //   if (buffer_map.count(id) == 1) {
    //     const Buffer& waypoint_deque = buffer_map[id];
    //     // Find the next intersection (if any) to decide to turn on the blinkers
    //     for (const SimpleWaypointPtr& swpp : waypoint_deque) {
    //       WaypointPtr wptr = swpp->GetWaypoint();
    //       cg::Vector3D road_vec = swpp->GetForwardVector();
    //       if (!wptr->IsJunction())
    //         continue;

    //       // Get the end of the junction road segment
    //       std::vector<WaypointPtr> next_wptrs = wptr -> GetNextUntilLaneEnd(2);
    //       if(next_wptrs.empty())
    //         break;
    //       wptr = next_wptrs.back();
    //       cg::Vector3D next_road_vec = wptr->GetTransform().GetForwardVector();
    //       cg::Vector3D up_vec(0, 0, 1);
    //       float dot_prod = actor_vec.x*next_road_vec.x + 
    //                        actor_vec.y*next_road_vec.y + 
    //                        actor_vec.z*next_road_vec.z;
    //       cg::Vector3D cross_prod(actor_vec.y*up_vec.z - actor_vec.z*up_vec.y,
    //                               actor_vec.z*up_vec.x - actor_vec.x*up_vec.z,
    //                               actor_vec.x*up_vec.y - actor_vec.y*up_vec.x);
          
    //       float dot_prod_left = cross_prod.x*next_road_vec.x + 
    //                             cross_prod.y*next_road_vec.y + 
    //                             cross_prod.z*next_road_vec.z;

    //       // Determine if the vehicle is truning left or right
    //       if(dot_prod < 0.5) {
    //         if(dot_prod_left > 0.5)
    //           left_turn_indicator = true;
    //         if(dot_prod_left < -0.5)
    //           right_turn_indicator = true;
    //       }
    //       break;  
    //     }
    //   }

    //   // Determine brake light state
    //   for (size_t cc = 0; cc < control_frame.size(); cc++) {
    //     if (control_frame[cc].command.type() == typeid(carla::rpc::Command::ApplyVehicleControl)) {
    //       carla::rpc::Command::ApplyVehicleControl& ctrl = boost::get<carla::rpc::Command::ApplyVehicleControl>(control_frame[cc].command);
    //       if (ctrl.actor == id) {
    //         brake_lights = (ctrl.control.brake > 0);
    //         break;
    //       }
    //     }
    //   }

    //   //determine position, fog and beams
      
    //   rpc::WeatherParameters wp = world.GetWeather();
    //   // std::cout << "sun_azimuth_angle: " << wp.sun_azimuth_angle << 
    //   //              " sun_altitude_angle: " << wp.sun_altitude_angle << 
    //   //              " fog_density: " << wp.fog_density << 
    //   //              " precipitation: " << wp.precipitation << std::endl;

    //   // turn on beams & positions from sunset to dawn
    //   if (wp.sun_altitude_angle < 15 || wp.sun_altitude_angle > 165) {
    //     position = true;
    //     low_beam = true;
    //   }
    //   else if (wp.sun_altitude_angle < 35 || wp.sun_altitude_angle > 145) {
    //     position = true;
    //   }
    //   // turn on lights under heavy rain
    //   if (wp.precipitation > 80) {
    //     position = true;
    //     low_beam = true;
    //   }
    //   // turn on fog lights
    //   if (wp.fog_density > 20) {
    //     position = true;
    //     low_beam = true;
    //     fog_lights = true;
    //   }

    //   // Determine the new vehicle light state
    //   rpc::VehicleLightState::flag_type new_light_states = light_states;
    //   if (brake_lights)
    //     new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Brake);
    //   else
    //     new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Brake);

    //   if (left_turn_indicator)
    //     new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LeftBlinker);
    //   else
    //     new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LeftBlinker);

    //   if (right_turn_indicator)
    //     new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::RightBlinker);
    //   else
    //     new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::RightBlinker);
      
    //   if (position)
    //     new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Position);
    //   else
    //     new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Position);
        
    //   if (low_beam)
    //     new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LowBeam);
    //   else
    //     new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LowBeam);

    //   if (high_beam)
    //     new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::HighBeam);
    //   else
    //     new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::HighBeam);
      
    //   if (fog_lights)
    //     new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Fog);
    //   else
    //     new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Fog);
      
    //   // Update the vehicle light state if it has changed
    //   if (new_light_states != light_states)
    //     control_frame.push_back(carla::rpc::Command::SetVehicleLightState(id, new_light_states));
    //   appliedLightStateCount++;
    // }
    // std::cout << "Applying light states to "<<appliedLightStateCount<<" actors..." << std::endl;
    ///////////////////////////////////////////////

    registration_lock.unlock();

    // Sending the current cycle's batch command to the simulator.
    if (synchronous_mode) {
      episode_proxy.Lock()->ApplyBatchSync(control_frame, false);
      step_end.store(true);
      step_end_trigger.notify_one();
    } else {
      if (control_frame.size() > 0){
        episode_proxy.Lock()->ApplyBatchSync(control_frame, false);
      }
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
  if (parameters.GetSynchronousMode()) {
    step_begin_trigger.notify_one();
  }

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
  random_devices.clear();

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
  std::lock_guard<std::mutex> registration_lock(registration_mutex);
  registered_vehicles.Insert(vehicle_list);
  for (const ActorPtr &vehicle: vehicle_list) {
    if (!is_custom_seed) {
      seed = vehicle->GetId() + seed;
    } else {
      seed = 1 + seed;
    }
    random_devices.insert({vehicle->GetId(), RandomGenerator(seed)});
  }
}

void TrafficManagerLocal::UnregisterVehicles(const std::vector<ActorPtr> &actor_list) {
  std::lock_guard<std::mutex> registration_lock(registration_mutex);
  std::vector<ActorId> actor_id_list;
  for (auto &actor : actor_list) {
    alsm.RemoveActor(actor->GetId(), true);
  }
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

void TrafficManagerLocal::SetOSMMode(const bool mode_switch) {
  parameters.SetOSMMode(mode_switch);
}

void TrafficManagerLocal::SetRespawnDormantVehicles(const bool mode_switch) {
  parameters.SetRespawnDormantVehicles(mode_switch);
}

void TrafficManagerLocal::SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) {
  parameters.SetBoundariesRespawnDormantVehicles(lower_bound, upper_bound);
}

void TrafficManagerLocal::SetMaxBoundaries(const float lower, const float upper) {
  parameters.SetMaxBoundaries(lower, upper);
}

bool TrafficManagerLocal::CheckAllFrozen(TLGroup tl_to_freeze) {
  for (auto &elem : tl_to_freeze) {
    if (!elem->IsFrozen() || elem->GetState() != TLS::Red) {
      return false;
    }
  }
  return true;
}

void TrafficManagerLocal::SetSynchronousMode(bool mode) {
  const bool previous_mode = parameters.GetSynchronousMode();
  parameters.SetSynchronousMode(mode);
  if (previous_mode && !mode) {
    step_begin.store(true);
    step_begin_trigger.notify_one();
  }
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

void TrafficManagerLocal::SetRandomDeviceSeed(const uint64_t _seed) {
  seed = _seed;
  is_custom_seed = true;
  world.ResetAllTrafficLights();
}

} // namespace traffic_manager
} // namespace carla
