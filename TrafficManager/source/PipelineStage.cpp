// Definition file for members of class PipelineStage

#include "PipelineStage.h"

namespace traffic_manager {

  PipelineStage::PipelineStage(int pool_size): pool_size(pool_size) {
    run_stage = true;
  }

  PipelineStage::~PipelineStage() {}

  void PipelineStage::Start() {
    data_reciever = std::make_unique<std::thread>(DataReciever);
    for (int i=0; i<pool_size; i++) {
      action_threads.push_back(std::make_unique<std::thread>(Action, i));
    }
    data_sender = std::make_unique<std::thread>(DataSender);
  }

  void PipelineStage::Stop() {
    run_stage = false;
  }

}
