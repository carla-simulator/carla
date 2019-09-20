#include "BatchControlStage.h"

namespace traffic_manager {

  BatchControlStage::BatchControlStage(
      std::shared_ptr<PlannerToControlMessenger> messenger,
      carla::client::Client &carla_client,
      int number_of_vehicles,
      int pool_size)
    : messenger(messenger),
      carla_client(carla_client),
      PipelineStage(pool_size, number_of_vehicles) {

    // Initializing messenger state
    messenger_state = messenger->GetState();
    // Initializing throuput count
    frame_count = 0;
    // Initializing clock for throughput measurement
    last_update_instance = std::chrono::system_clock::now();
    // Allocating array for command batching
    commands = std::make_shared<std::vector<carla::rpc::Command>>(number_of_vehicles);
  }

  BatchControlStage::~BatchControlStage() {}

  void BatchControlStage::Action(const int start_index, const int end_index) {

    // Looping over arrays' partitions for current thread
    for (int i = start_index; i <= end_index; ++i) {

      carla::rpc::VehicleControl vehicle_control;

      auto &element = data_frame->at(i);
      auto actor_id = element.actor_id;
      vehicle_control.throttle = element.throttle;
      vehicle_control.brake = element.brake;
      vehicle_control.steer = element.steer;

      commands->at(i) = carla::rpc::Command::ApplyVehicleControl(actor_id, vehicle_control);
    }
  }

  void BatchControlStage::DataReceiver() {

    auto packet = messenger->ReceiveData(messenger_state);
    data_frame = packet.data;
    messenger_state = packet.id;
  }

  void BatchControlStage::DataSender() {

    carla_client.ApplyBatch(*commands.get());

    // Measuring throughput
    auto current_time = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = current_time - last_update_instance;
    ++frame_count;
    if (diff.count() > 1.0) {
      carla::log_info("Processed " + std::to_string(frame_count) + " frames per second\n");
      last_update_instance = current_time;
      frame_count = 0;
    }

    // limiting updates to 100 frames per second
    std::this_thread::sleep_for(10ms);  
  }
}
