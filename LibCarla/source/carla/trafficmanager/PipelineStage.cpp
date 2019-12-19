// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "PipelineStage.h"

namespace carla {
namespace traffic_manager {

  PipelineStage::PipelineStage(std::string stage_name)
    : stage_name(stage_name),
      performance_diagnostics(PerformanceDiagnostics(stage_name)) {
    run_stage.store(false);
  }

  PipelineStage::~PipelineStage() {
    worker_thread->join();
    worker_thread.release();
  }

  void PipelineStage::Start() {
    run_stage.store(true);
    worker_thread = std::make_unique<std::thread>(&PipelineStage::Update, this);
  }

  void PipelineStage::Stop() {
    run_stage.store(false);
  }

  void PipelineStage::Update() {
    while (run_stage.load()){
      // Receive data.
      DataReceiver();

      // Receive data.
      if(run_stage.load()){
        performance_diagnostics.RegisterUpdate(true);
        Action();
        performance_diagnostics.RegisterUpdate(false);
      }

      // Receive data.
      if(run_stage.load()) {
        DataSender();
      }

    }
  }

} // namespace traffic_manager
} // namespace carla
