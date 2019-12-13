// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "PipelineStage.h"

namespace carla {
namespace traffic_manager {

  PipelineStage::PipelineStage(std::string stage_name)
    : performance_diagnostics(PerformanceDiagnostics(stage_name)) {

    run_stage.store(true);
    run_receiver.store(true);
    run_action.store(false);
    run_sender.store(false);
  }

  PipelineStage::~PipelineStage() {}

  void PipelineStage::Start() {

    data_receiver = std::make_unique<std::thread>(&PipelineStage::ReceiverThreadManager, this);
    action_thread = std::make_unique<std::thread>(&PipelineStage::ActionThreadManager, this);
    data_sender = std::make_unique<std::thread>(&PipelineStage::SenderThreadManager, this);
  }

  void PipelineStage::Stop() {

    run_stage.store(false);
    data_receiver->join();
    action_thread->join();
    data_sender->join();
  }

  void PipelineStage::ReceiverThreadManager() {

    while (run_stage.load()) {
      std::unique_lock<std::mutex> lock(thread_coordination_mutex);
      // Wait for notification from sender thread and
      // break waiting if the stage is stopped.
      while (!run_receiver.load() && run_stage.load()) {
        wake_receiver_notifier.wait_for(lock, 1ms, [=] {return run_receiver.load();});
      }
      lock.unlock();
      run_receiver.store(false);

      // Receive data.
      if (run_stage.load()) {
        DataReceiver();
      }

      // Notify action thread.
      run_action.store(true);
      wake_action_notifier.notify_one();
    }
  }

  void PipelineStage::ActionThreadManager() {

    while (run_stage.load()) {
      performance_diagnostics.RegisterUpdate();

      std::unique_lock<std::mutex> lock(thread_coordination_mutex);

      // Wait for notification from receiver thread.
      while (!run_action.load() && run_stage.load()) {
        wake_action_notifier.wait_for(lock, 1ms, [=] {return run_action.load();});
      }
      lock.unlock();
      run_action.store(false);

      // Run action.
      if (run_stage.load()) {
        Action();
      }

      // Notify sender.
      run_sender.store(true);
      wake_sender_notifier.notify_one();
    }
  }

  void PipelineStage::SenderThreadManager() {

    while (run_stage.load()) {
      std::unique_lock<std::mutex> lock(thread_coordination_mutex);

      // Wait for notification from action thread.
      while (!run_sender.load() && run_stage.load()) {
        wake_sender_notifier.wait_for(lock, 1ms, [=] {return run_sender.load();});
      }
      lock.unlock();
      run_sender.store(false);

      // Send data.
      if (run_stage.load()) {
        DataSender();
      }

      // Notify receiver.
      run_receiver.store(true);
      wake_receiver_notifier.notify_one();
    }
  }

} // namespace traffic_manager
} // namespace carla
