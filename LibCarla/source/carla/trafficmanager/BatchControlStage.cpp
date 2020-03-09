// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/BatchControlStage.h"

namespace carla {
namespace traffic_manager {

BatchControlStage::BatchControlStage(
      std::string stage_name,
      std::shared_ptr<PlannerToControlMessenger> messenger,
      carla::client::detail::EpisodeProxy &episode_proxy,
      Parameters &parameters)
    : PipelineStage(stage_name),
      messenger(messenger),
      episode_proxy_bcs(episode_proxy),
      parameters(parameters) {

  // Initializing number of vehicles to zero in the beginning.
  number_of_vehicles = 0u;
}

BatchControlStage::~BatchControlStage() {}

void BatchControlStage::Action() {

  // Looping over registered actors.
  for (uint64_t i = 0u; i < number_of_vehicles && data_frame != nullptr; ++i) {

    carla::rpc::VehicleControl vehicle_control;

      const PlannerToControlData &element = data_frame->at(i);
      if (!element.actor || !element.actor->IsAlive()) {
        continue;
      }
      const carla::ActorId actor_id = element.actor->GetId();

      vehicle_control.throttle = element.throttle;
      vehicle_control.brake = element.brake;
      vehicle_control.steer = element.steer;

    commands->at(i) = carla::rpc::Command::ApplyVehicleControl(actor_id, vehicle_control);
  }
}

void BatchControlStage::DataReceiver() {

  data_frame = messenger->Peek();

  // Allocating new containers for the changed number of registered vehicles.
  if (data_frame != nullptr && number_of_vehicles != (*data_frame.get()).size()) {

    number_of_vehicles = static_cast<uint64_t>((*data_frame.get()).size());

    // Allocating array for command batching.
    commands = std::make_shared<std::vector<carla::rpc::Command>>(number_of_vehicles);
  }
}

void BatchControlStage::DataSender() {

  messenger->Pop();
  bool synch_mode = parameters.GetSynchronousMode();

  // Asynchronous mode.
  if (!synch_mode) {
    // Apply batch command through an asynchronous RPC call.
    if (commands != nullptr) {
      episode_proxy_bcs.Lock()->ApplyBatch(*commands.get(), false);
    }
    // Limiting updates to 100 frames per second.
    std::this_thread::sleep_for(10ms);
  }
  // Synchronous mode.
  else {
    std::unique_lock<std::mutex> lock(step_execution_mutex);
    // TODO: Re-introduce timeout while waiting for RunStep() call.
    while (!run_step.load()) {
      step_execution_notifier.wait_for(lock, 1ms, [this]() {return run_step.load();});
    }
    // Apply batch command in synchronous RPC call.
    if (commands != nullptr) {
      episode_proxy_bcs.Lock()->ApplyBatchSync(*commands.get(), false);
    }
    // Set flag to false, unblock RunStep() call and release mutex lock.
    run_step.store(false);
    step_complete_notifier.notify_one();
    lock.unlock();
  }
}


bool BatchControlStage::RunStep() {

  bool synch_mode = parameters.GetSynchronousMode();
  if (synch_mode) {
    std::unique_lock<std::mutex> lock(step_execution_mutex);
    // Set flag to true, notify DataSender and wait for a return notification
    run_step.store(true);
    step_execution_notifier.notify_one();
    while (run_step.load()) {
      step_complete_notifier.wait_for(lock, 1ms, [this]() {return !run_step.load();});
    }
  }

  return true;
}

} // namespace traffic_manager
} // namespace carla
