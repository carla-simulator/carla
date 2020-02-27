// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/PipelineStage.h"

namespace carla {
namespace traffic_manager {

PipelineStage::PipelineStage(
    const std::string &stage_name)
  : stage_name(stage_name),
    performance_diagnostics(PerformanceDiagnostics(stage_name)) {
  run_stage.store(false);
}

PipelineStage::~PipelineStage() {
  Stop();
}

void PipelineStage::Start() {
  run_stage.store(true);
  if(worker_thread) {
    Stop();
  }
  worker_thread = std::make_unique<std::thread>(&PipelineStage::Update, this);
}

void PipelineStage::Stop() {
  run_stage.store(false);
  if(worker_thread) {
    if(worker_thread->joinable()){
      worker_thread->join();
    }
    worker_thread.release();
  }
}

void PipelineStage::Update() {
  while (run_stage.load()){
    // Receive data.
    DataReceiver();

    if(run_stage.load()){
      performance_diagnostics.RegisterUpdate(true);
      Action();
      performance_diagnostics.RegisterUpdate(false);
    }

    if(run_stage.load()) {
      DataSender();
    }

  }
}

} // namespace traffic_manager
} // namespace carla
