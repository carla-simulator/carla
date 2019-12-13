// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
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

    /// Pointer to receiver thread instance.
    std::unique_ptr<std::thread> data_receiver;
    /// Pointer to sender thread instance.
    std::unique_ptr<std::thread> data_sender;
    /// Pointer to worker thread instance.
    std::unique_ptr<std::thread> action_thread;
    /// Flag to allow/block receiver.
    std::atomic<bool> run_receiver;
    /// Flag to allow/block sender.
    std::atomic<bool> run_sender;
    /// Flag to allow/block workers.
    std::atomic<bool> run_action;
    /// Flag to start/stop stage.
    std::atomic<bool> run_stage;
    /// Mutex used to co-ordinate between receiver, workers, and sender.
    std::mutex thread_coordination_mutex;
    /// Variables to conditionally block receiver, workers, and sender.
    std::condition_variable wake_receiver_notifier;
    std::condition_variable wake_action_notifier;
    std::condition_variable wake_sender_notifier;
    /// Object to track stage performance.
    PerformanceDiagnostics performance_diagnostics;

    /// Method to manage receiver thread.
    void ReceiverThreadManager();

    /// Method to manage worker threads.
    void ActionThreadManager();

    /// Method to manage sender thread.
    void SenderThreadManager();

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

    PipelineStage(std::string stage_name);

    virtual ~PipelineStage();

    void Start();

    void Stop();

  };

} // namespace traffic_manager
} // namespace carla
