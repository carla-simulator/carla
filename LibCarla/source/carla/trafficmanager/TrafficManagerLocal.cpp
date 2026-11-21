// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <algorithm>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

#include "carla/Exception.h"
#include "carla/Logging.h"

#include "carla/client/detail/Simulator.h"

#include "carla/trafficmanager/TrafficManager.h"
#include "carla/trafficmanager/TrafficManagerLocal.h"

namespace carla {
namespace traffic_manager {

using namespace constants::FrameMemory;
using namespace constants::WorldInfoRefresh;

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
                                         large_vehicles,
                                         parameters,
                                         marked_for_removal,
                                         localization_frame,
                                         random_device)),

    collision_stage(CollisionStage(vehicle_id_list,
                                   simulation_state,
                                   buffer_map,
                                   track_traffic,
                                   parameters,
                                   collision_frame,
                                   random_device)),

    traffic_light_stage(TrafficLightStage(vehicle_id_list,
                                          simulation_state,
                                          buffer_map,
                                          parameters,
                                          world,
                                          tl_frame,
                                          random_device)),

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
                                      control_frame,
                                      random_device,
                                      local_map,
                                      large_vehicles)),

    vehicle_light_stage(VehicleLightStage(vehicle_id_list,
                                          buffer_map,
                                          parameters,
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
              large_vehicles,
              localization_stage,
              collision_stage,
              traffic_light_stage,
              motion_plan_stage,
              vehicle_light_stage,
              update_batch)),

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
      if (!local_map->Load(content)) {
        // The copy in the client's file cache (~/carlaCache) is kept for ever once
        // downloaded, so a rejected cache is most likely a stale local copy: fetch
        // the server's current file once and try again before rebuilding from the
        // OpenDRIVE.
        log_warning("fetching the Traffic Manager cache", files[0], "from the server again");
        episode_proxy.Lock()->RequestFile(files[0]);
        content = episode_proxy.Lock()->GetCacheFile(files[0], false);
        local_map = std::make_shared<InMemoryMap>(world_map);
        if (content.size() == 0 || !local_map->Load(content)) {
          local_map = std::make_shared<InMemoryMap>(world_map);
          local_map->SetUp();
        }
      }
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

  localization_frame.reserve(INITIAL_SIZE);
  collision_frame.reserve(INITIAL_SIZE);
  tl_frame.reserve(INITIAL_SIZE);
  control_frame.reserve(INITIAL_SIZE);
  current_reserved_capacity = INITIAL_SIZE;
  
  bool synchronous_mode = parameters.GetSynchronousMode();
  if (!synchronous_mode) {
    worker_thread = std::make_unique<std::thread>(&TrafficManagerLocal::Run, this);
  }
}

/// Adds the stages' commands to @a batch. The batch applies each kind in a
/// fixed order (transforms, then controls, then light states), which matches
/// the frame's order: every vehicle gets either a transform or a control, and
/// the light states come after both.
static void AddControlFrame(const ControlFrame &frame, carla::rpc::ActorUpdateBatchBuilder &batch) {
  namespace cmd = carla::rpc;
  for (const auto &command : frame) {
    std::visit([&batch](const auto &c) {
      using T = std::decay_t<decltype(c)>;
      if constexpr (std::is_same_v<T, cmd::Command::ApplyVehicleControl>) {
        batch.AddVehicleControl(
            c.actor, c.control.throttle, c.control.steer, c.control.brake,
            c.control.hand_brake, c.control.reverse, c.control.manual_gear_shift, c.control.gear);
      } else if constexpr (std::is_same_v<T, cmd::Command::ApplyTransform>) {
        const auto &l = c.transform.location;
        const auto &r = c.transform.rotation;
        const double location[3] = {l.x, l.y, l.z};
        const float rotation[3] = {r.pitch, r.yaw, r.roll};
        batch.AddTransform(c.actor, location, rotation);
      } else if constexpr (std::is_same_v<T, cmd::Command::SetVehicleLightState>) {
        batch.AddVehicleLightState(c.actor, c.light_state);
      } else {
        // The stages emit only the three commands above.
        log_error("Traffic Manager: unexpected command in the control frame, dropped");
      }
    }, command.command);
  }
}

void TrafficManagerLocal::Step() {
  bool synchronous_mode = parameters.GetSynchronousMode();
  bool hybrid_physics_mode = parameters.GetHybridPhysicsMode();

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

  // Read from the episode state the client already holds, so no round trip.
  const carla::client::Timestamp timestamp = world.GetSnapshot().GetTimestamp();

  // Stop TM from processing the same frame more than once
  if (!synchronous_mode) {
    if (timestamp.frame == last_frame) {
      std::this_thread::sleep_for(SNAPSHOT_POLL_PERIOD);
      return;
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
  // Reserve two frames for each vehicle: one for the ApplyVehicleControl command,
  // and one for the optional SetVehicleLightState command
  control_frame.reserve(2 * number_of_vehicles);
  // Resize to accomodate at least all ApplyVehicleControl commands,
  // that will be inserted by the motion_plan_stage stage.
  control_frame.resize(number_of_vehicles);

  // Run core operation stages.
  for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index) {
    localization_stage.Update(index);
  }
  for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index) {
    if (!localization_frame[index].localized) {
      continue;
    }
    collision_stage.Update(index);
  }
  collision_stage.ClearCycleCache();
  vehicle_light_stage.UpdateWorldInfo(timestamp.elapsed_seconds, synchronous_mode);
  motion_plan_stage.SetCycleTimestamp(timestamp);
  for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index) {
    if (!localization_frame[index].localized) {
      continue;
    }
    traffic_light_stage.Update(index);
    motion_plan_stage.Update(index);
    vehicle_light_stage.Update(index);
  }

  // A vehicle skipped above never had its control command written, and the
  // default-constructed command would reach the server as an empty spawn
  // request ("Invalid ActorDescription '' (UId=0)"). Drop those slots; the
  // commands appended past the per-vehicle range (vehicle lights) are kept.
  const bool any_skipped = std::any_of(
      localization_frame.begin(),
      localization_frame.begin() + static_cast<long>(vehicle_id_list.size()),
      [](const LocalizationData &data) { return !data.localized; });
  if (any_skipped) {
    ControlFrame filtered_frame;
    filtered_frame.reserve(control_frame.size());
    for (unsigned long index = 0u; index < control_frame.size(); ++index) {
      if (index < vehicle_id_list.size() && !localization_frame[index].localized) {
        continue;
      }
      filtered_frame.push_back(std::move(control_frame[index]));
    }
    control_frame = std::move(filtered_frame);
  }

  AddControlFrame(control_frame, update_batch);
  registration_lock.unlock();

  // Sending the current cycle's updates to the simulator, as one blob after
  // the ALSM's physics toggles. Synchronous mode must apply them before the
  // next tick; async waiting costs a server frame.
  if (update_batch.empty()) {
    return;
  }
  auto simulator = episode_proxy.Lock();
  if (synchronous_mode && defer_updates_to_tick) {
    // Every tick of this process first waits until the server has queued
    // this call (the fence of a connection's own calls, or FenceUpdates).
    simulator->ApplyActorUpdatesAsync(update_batch.Finish());
    TrafficManager::AddUnfencedUpdates(simulator);
  } else if (synchronous_mode) {
    simulator->ApplyActorUpdates(update_batch.Finish(), false, simulator->GetNetworkingTimeout());
  } else {
    // An unwaited batch is acknowledged on receipt, not on application, so
    // periodically waiting is what bounds the queue ahead of the game thread.
    if (IsBatchSyncDue(unwaited_control_batches, MAX_UNWAITED_CONTROL_BATCHES)) {
      simulator->ApplyActorUpdates(update_batch.Finish(), false, simulator->GetNetworkingTimeout());
      unwaited_control_batches = 0u;
    } else {
      simulator->ApplyActorUpdatesAsync(update_batch.Finish());
      ++unwaited_control_batches;
    }
  }
}

void TrafficManagerLocal::Run() {
  while (run_traffic_manger.load() && !parameters.GetSynchronousMode()) {
    Step();
    // No TM lock held here: drop released vehicles.
    registered_vehicles.TakeReleased();
  }
}

bool TrafficManagerLocal::SynchronousTick() {
  if (parameters.GetSynchronousMode()) {
    Step();  // Do one step of TM.
  }
  return true;
}

bool TrafficManagerLocal::TickForFrame(uint64_t frame, const cc::detail::Simulator *ticker) {
  if (!parameters.GetSynchronousMode()) {
    return true;
  }
  // The client that ticked has the new frame, but this TM reads the world
  // through its own connection, which may still hold part of the previous one.
  auto simulator = episode_proxy.TryLock();
  if (simulator != nullptr && simulator.get() != ticker) {
    const bool arrived = simulator->WaitForFrame(frame, simulator->GetNetworkingTimeout());
    const uint64_t resyncs = simulator->GetSnapshotResyncCount();
    const bool resynced = (resyncs != snapshot_resyncs_seen);
    snapshot_resyncs_seen = resyncs;
    if (!arrived) {
      if (resynced) {
        // The connection dropped the snapshots it had and is waiting for a
        // whole one: the state it holds may be several frames old.
        throw_exception(std::runtime_error(
            "Traffic Manager: the world-snapshot stream of its connection "
            "resynchronized and frame " + std::to_string(frame) +
            " did not arrive; not stepping on an older world"));
      }
      log_warning("Traffic Manager: frame", frame, "did not arrive on its connection; stepping anyway");
    }
  }
  // Called from a tick, so the next tick of this process comes after this
  // step: the step need not wait for the server to apply its updates.
  defer_updates_to_tick = true;
  struct Restore {
    bool &flag;
    ~Restore() { flag = false; }
  } restore{defer_updates_to_tick};
  Step();
  return true;
}

void TrafficManagerLocal::Stop() {
  run_traffic_manger.store(false);

  // Join regardless of mode, or Start() may overwrite a joinable thread.
  if (worker_thread) {
    if (worker_thread->joinable()) {
      worker_thread->join();
    }
    worker_thread.reset();
  }

  std::unique_lock<std::mutex> registration_lock(registration_mutex);
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
  vehicle_light_stage.Reset();

  last_frame = 0;
  unwaited_control_batches = MAX_UNWAITED_CONTROL_BATCHES;

  buffer_map.clear();
  localization_frame.clear();
  collision_frame.clear();
  tl_frame.clear();
  control_frame.clear();
  update_batch.Clear();
  registration_lock.unlock();

  run_traffic_manger.store(true);
}

void TrafficManagerLocal::Release() {

  Stop();

  local_map.reset();
}

void TrafficManagerLocal::Reset(uint64_t episode_id) {
  // Every client alive in the process reports the change; reset once.
  if (episode_proxy.GetId() == episode_id) {
    return;
  }
  // Our client may see the change after another one; wait, or we'd reset onto the old episode.
  auto simulator = episode_proxy.Lock();
  const auto deadline = std::chrono::steady_clock::now() + simulator->GetNetworkingTimeout().to_chrono();
  while (simulator->GetCurrentEpisodeId() != episode_id && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  Release();
  episode_proxy = simulator->GetCurrentEpisode();
  world = cc::World(episode_proxy);
  SetupLocalMap();
  Start();
}

void TrafficManagerLocal::RegisterVehicles(const std::vector<ActorPtr> &vehicle_list) {
  std::scoped_lock<std::mutex> registration_lock(registration_mutex);
  registered_vehicles.Insert(vehicle_list);
  for (auto actor : vehicle_list) {
    alsm.AddActor(actor);
  }
}

void TrafficManagerLocal::UnregisterVehicles(const std::vector<ActorPtr> &actor_list) {
  std::scoped_lock<std::mutex> registration_lock(registration_mutex);
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

void TrafficManagerLocal::SetLaneOffset(const ActorPtr &actor, const float offset) {
  parameters.SetLaneOffset(actor, offset);
}

void TrafficManagerLocal::SetGlobalLaneOffset(const float offset) {
  parameters.SetGlobalLaneOffset(offset);
}

void TrafficManagerLocal::SetLargeVehicleWideTurn(const ActorPtr &actor, const bool enable) {
  parameters.SetLargeVehicleWideTurn(actor, enable);
}

void TrafficManagerLocal::SetGlobalLargeVehicleWideTurn(const bool enable) {
  parameters.SetGlobalLargeVehicleWideTurn(enable);
}

void TrafficManagerLocal::SetDesiredSpeed(const ActorPtr &actor, const float value) {
  parameters.SetDesiredSpeed(actor, value);
}

/// Method to set the automatic management of the vehicle lights
void TrafficManagerLocal::SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update) {
  parameters.SetUpdateVehicleLights(actor, do_update);
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

void TrafficManagerLocal::SetKeepSlowLanePercentage(const ActorPtr &actor, const float percentage) {
  parameters.SetKeepSlowLanePercentage(actor, percentage);
}


void TrafficManagerLocal::SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage) {
  parameters.SetRandomLeftLaneChangePercentage(actor, percentage);
}

void TrafficManagerLocal::SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage) {
  parameters.SetRandomRightLaneChangePercentage(actor, percentage);
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

void TrafficManagerLocal::SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer) {
  parameters.SetCustomPath(actor, path, empty_buffer);
}

void TrafficManagerLocal::RemoveUploadPath(const ActorId &actor_id, const bool remove_path) {
  parameters.RemoveUploadPath(actor_id, remove_path);
}

void TrafficManagerLocal::UpdateUploadPath(const ActorId &actor_id, const Path path) {
  parameters.UpdateUploadPath(actor_id, path);
}

void TrafficManagerLocal::SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer) {
  parameters.SetImportedRoute(actor, route, empty_buffer);
}

void TrafficManagerLocal::RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) {
  parameters.RemoveImportedRoute(actor_id, remove_path);
}

void TrafficManagerLocal::UpdateImportedRoute(const ActorId &actor_id, const Route route) {
  parameters.UpdateImportedRoute(actor_id, route);
}

Action TrafficManagerLocal::GetNextAction(const ActorId &actor_id) {
  return localization_stage.ComputeNextAction(actor_id);
}

ActionBuffer TrafficManagerLocal::GetActionBuffer(const ActorId &actor_id) {
  return localization_stage.ComputeActionBuffer(actor_id);
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
  const bool previously_synchronous = parameters.GetSynchronousMode();
  parameters.SetSynchronousMode(mode);
  if (mode == previously_synchronous) { // Same mode as before, do nothing.
    return;
  }

  if (!previously_synchronous) {  // Switch from async to synchronous
    if (worker_thread) {
      if (worker_thread->joinable()) {
        worker_thread->join();
      }
      worker_thread.reset();
    }
  }
  else { // Switch from synchronous to async, start worker thread again.
    if (worker_thread) {
      // A worker thread should not exist while in synchronous mode. Tear it
      // down before spawning a fresh one so a buggy prior transition does
      // not leak running threads.
      log_error("TrafficManagerLocal: worker thread present in synchronous mode; joining before async restart.");
      if (worker_thread->joinable()) {
        worker_thread->join();
      }
      worker_thread.reset();
    }
    worker_thread = std::make_unique<std::thread>(&TrafficManagerLocal::Run, this);
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
  random_device = RandomGenerator(seed);
  world.ResetAllTrafficLights();
}

} // namespace traffic_manager
} // namespace carla
