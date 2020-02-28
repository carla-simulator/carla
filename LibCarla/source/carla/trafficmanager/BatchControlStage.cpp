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

  if (commands != nullptr) {
    // Run asynchronous mode commands.
    episode_proxy_bcs.Lock()->ApplyBatch(*commands.get(), false);
  }

  // Limiting updates to 100 frames per second.
  if (!synch_mode) {
    std::this_thread::sleep_for(10ms);
  } else {
    std::unique_lock<std::mutex> lock(step_execution_mutex);
    // Get timeout value in milliseconds.
    double timeout = parameters.GetSynchronousModeTimeOutInMiliSecond();
    // Wait for service to finish.
    step_execution_notifier.wait_for(lock, timeout * 1ms, [this]() { return run_step.load(); });
    run_step.store(false);
  }
}


bool BatchControlStage::RunStep() {
  // Set run set flag.
  run_step.store(true);

  return true;
}

} // namespace traffic_manager
} // namespace carla
