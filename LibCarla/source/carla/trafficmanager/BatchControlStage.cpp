// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "BatchControlStage.h"

namespace carla
{
namespace traffic_manager
{

BatchControlStage::BatchControlStage(std::string stage_name,
                                     std::shared_ptr<PlannerToControlMessenger> messenger,
                                     carla::client::detail::EpisodeProxy &episodeProxy,
                                     Parameters &parameters)
    : PipelineStage(stage_name),
      messenger(messenger),
      episodeProxyBCS(episodeProxy),
      parameters(parameters)
{

  // Initializing number of vehicles to zero in the beginning.
  number_of_vehicles = 0u;
}

BatchControlStage::~BatchControlStage() {}

void BatchControlStage::Action()
{

  // Looping over registered actors.
  for (uint64_t i = 0u; i < number_of_vehicles && data_frame != nullptr; ++i)
  {

    carla::rpc::VehicleControl vehicle_control;

    const PlannerToControlData &element = data_frame->at(i);
    const carla::ActorId actor_id = element.actor_id;
    vehicle_control.throttle = element.throttle;
    vehicle_control.brake = element.brake;
    vehicle_control.steer = element.steer;

    commands->at(i) = carla::rpc::Command::ApplyVehicleControl(actor_id, vehicle_control);
  }
}

void BatchControlStage::DataReceiver()
{

  data_frame = messenger->Peek();

  // Allocating new containers for the changed number of registered vehicles.
  if (data_frame != nullptr &&
      number_of_vehicles != (*data_frame.get()).size())
  {

    number_of_vehicles = static_cast<uint64_t>((*data_frame.get()).size());
    // Allocating array for command batching.
    commands = std::make_shared<std::vector<carla::rpc::Command>>(number_of_vehicles);
  }
}

void BatchControlStage::DataSender()
{

  messenger->Pop();
  bool synch_mode = parameters.GetSynchronousMode();

  if (commands != nullptr)
  {
    episodeProxyBCS.Lock()->ApplyBatch(*commands.get(), false);
    if (synch_mode)
    {
      std::unique_lock<std::mutex> lock(step_execution_mutex);
      while (!run_step.load())
      {
        send_control_notifier.wait_for(lock, 1ms, [](std::atomic<bool> &run_step) { return run_step.load(); });
      }
      episodeProxyBCS.Lock()->ApplyBatchSync(*commands.get(), false);
      run_step.store(false);
      step_execution_notifier.notify_one();
    }
  }

  // Limiting updates to 100 frames per second.
  if (!synch_mode)
  {
    std::this_thread::sleep_for(10ms);
  }
}

void BatchControlStage::RunStep()
{
  std::unique_lock<std::mutex> lock(step_execution_mutex);
  run_step.store(true);
  send_control_notifier.notify_one();
  while (run_step.load())
  {
    step_execution_notifier.wait_for(lock, 1ms, [](std::atomic<bool> &run_step) { return !run_step.load(); });
  }
}

} // namespace traffic_manager
} // namespace carla
