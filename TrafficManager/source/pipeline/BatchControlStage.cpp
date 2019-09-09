// Definition of class memebers

#include "BatchControlStage.h"

namespace traffic_manager {

  BatchControlStage::BatchControlStage(
    std::shared_ptr<PlannerToControlMessenger> messenger,
    carla::client::Client& carla_client,
    int number_of_vehicles,
    int pool_size
  ):
    messenger(messenger),
    carla_client(carla_client),
    PipelineStage(pool_size, number_of_vehicles)
  {
    messenger_state = messenger->GetState();
    frame_count = 0;
    last_update_instance = std::chrono::system_clock::now();
    commands = std::make_shared<std::vector<carla::rpc::Command>>(number_of_vehicles);
  }

  BatchControlStage::~BatchControlStage() {}

  void BatchControlStage::Action(int start_index, int end_index) {

    for (int i=start_index; i<=end_index; i++) {

      carla::rpc::VehicleControl vehicle_control;

      auto& element = data_frame->at(i);
      auto actor_id = element.actor_id;
      vehicle_control.throttle = element.throttle;
      vehicle_control.brake = element.brake;
      vehicle_control.steer = element.steer;

      carla::rpc::Command::ApplyVehicleControl control_command(actor_id, vehicle_control);

      auto& command_reference = commands->at(i);
      command_reference = control_command;
    }
  }

  void BatchControlStage::DataReceiver() {

    auto packet = messenger->ReceiveData(messenger_state);
    data_frame = packet.data;
    messenger_state = packet.id;
  }

  void BatchControlStage::DataSender() {

    carla_client.ApplyBatch(*commands.get());

    auto current_time = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = current_time - last_update_instance;

    frame_count++;
    if (diff.count() > 1.0) {
      std::cout << "Processed " << frame_count << " frames per second" << std::endl;
      last_update_instance = current_time;
      frame_count = 0;
    }

    // std::this_thread::sleep_for(10ms);
  }
}
