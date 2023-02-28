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

    // Apply actuation from controller if physics enabled.
    if (element.physics_enabled) {
      vehicle_control.throttle = element.throttle;
      vehicle_control.brake = element.brake;
      vehicle_control.steer = element.steer;

      commands->at(i) = carla::rpc::Command::ApplyVehicleControl(actor_id, vehicle_control);
    }
    // Apply target transform for physics-less vehicles.
    else {

      const cg::Transform vehicle_transform = element.actor->GetTransform();
      const cg::Location vehicle_location = vehicle_transform.location;
      const cg::Vector3D vehicle_heading = vehicle_transform.GetForwardVector();
      const cg::Transform &teleportation_transform = element.transform;
      const cg::Location &teleportation_location_candidate = teleportation_transform.location;
      const cg::Vector3D teleprotation_relative_position = teleportation_location_candidate - vehicle_location;

      cg::Transform corrected_teleportation_transform = teleportation_transform;
      // Validate if teleportation transform is ahead of the vehicle's current position.
      // If not, then teleport to vehicle's current position.
      if (cg::Math::Dot(teleprotation_relative_position, vehicle_heading) < 0.0f) {
        corrected_teleportation_transform = vehicle_transform;
      }

      commands->at(i) = carla::rpc::Command::ApplyTransform(actor_id, corrected_teleportation_transform);
    }
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
    // Applying an infinitesimal sleep statement for providing a system cancellation point.
    std::this_thread::sleep_for(1us);
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
      episode_proxy_bcs.Lock()->ApplyBatchSync(*commands.get(), true);
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
