// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

#include "carla/Logging.h"
#include "carla/rpc/ActorId.h"

#include "carla/trafficmanager/Messenger.h"
#include "carla/trafficmanager/PerformanceDiagnostics.h"

namespace carla {
namespace traffic_manager {

  namespace chr = std::chrono;
  using namespace std::chrono_literals;

  /// This class provides base functionality and template for
  /// various stages of the pipeline.
  class PipelineStage {

  private:
    std::unique_ptr<std::thread> worker_thread;

  protected:
    /// Flag to start/stop stage.
    std::atomic<bool> run_stage;
    /// Stage name string.
    std::string stage_name;

  private:
    /// Object to track stage performance.
    PerformanceDiagnostics performance_diagnostics;

    void Update();

  protected:

    /// Implement this method with the logic to receive data from
    /// the previous stage(s) and distribute to Action() threads.
    virtual void DataReceiver() = 0;

    /// Implement this method with logic to gather results from the
    /// action threads and send to next stage(s).
    virtual void DataSender() = 0;

    /// Implement this method with logic to process data inside the stage
    virtual void Action() = 0;

  public:

    PipelineStage(const std::string &stage_name);

    virtual ~PipelineStage();

    void Start();

    void Stop();

  };

} // namespace traffic_manager
} // namespace carla
