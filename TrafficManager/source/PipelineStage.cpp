#include "PipelineStage.h"

namespace traffic_manager {

  PipelineStage::PipelineStage (
    int pool_size,
    int number_of_vehicles
  ): pool_size(pool_size), number_of_vehicles(number_of_vehicles) {
    action_counter = 0;
    run_stage = true;
    run_receiver = true;
    run_threads = false;
    run_sender = false;
  }

  PipelineStage::~PipelineStage() {}

  void PipelineStage::Start() {
    data_receiver = std::make_shared<std::thread>(&PipelineStage::DataReceiver, this);
    for (int i=0; i<pool_size; i++) {
      action_threads.push_back(std::make_shared<std::thread>(&PipelineStage::Action, this, i));
    }
    data_sender = std::make_shared<std::thread>(&PipelineStage::DataSender, this);
  }

  void PipelineStage::Stop() {
    run_stage.store(false);
  }

}
