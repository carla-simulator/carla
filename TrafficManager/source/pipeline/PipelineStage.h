#pragma once

#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include <thread>
#include <vector>

#include "carla/rpc/ActorId.h"

#include "Messenger.h"

using namespace std::chrono_literals;

namespace traffic_manager {

  /// This class provides base functionality and template for
  /// Various stages of the pipeline
  class PipelineStage {

  private:

    /// Number of worker threads
    const uint pool_size;
    /// Number of registered vehicles
    const uint number_of_vehicles;
    /// Pointer to receiver thread instance
    std::unique_ptr<std::thread> data_receiver;
    /// Pointer to sender thread instance
    std::unique_ptr<std::thread> data_sender;
    /// Pointers to worker thread instances
    std::vector<std::unique_ptr<std::thread>> action_threads;
    /// Counter to track every worker's start condition
    std::atomic<uint> action_start_counter;
    /// Counter to track every worker's finish condition
    std::atomic<uint> action_finished_counter;
    /// Flag to allow/block receiver
    std::atomic<bool> run_receiver;
    /// Flag to allow/block sender
    std::atomic<bool> run_sender;
    /// Flag to allow/block workers
    std::atomic<bool> run_threads;
    /// Flag to start/stop stage
    std::atomic<bool> run_stage;
    /// Mutex used to co-ordinate between receiver, workers and sender
    std::mutex thread_coordination_mutex;
    /// Variables to conditionally block receiver, workers and sender
    std::condition_variable wake_receiver_notifier;
    std::condition_variable wake_action_notifier;
    std::condition_variable wake_sender_notifier;

    /// Method to manage receiver thread
    void ReceiverThreadManager();

    /// Method to manage worker threads
    void ActionThreadManager(const uint thread_id);

    /// Method to manage sender thread
    void SenderThreadManager();

  protected:

    /// Implement this method with the logic to receive data from
    /// Previous stage(s) and distribute to Action() threads
    virtual void DataReceiver() = 0;

    /// Implement this method with logic to gather results from action
    /// Threads and send to next stage(s)
    virtual void DataSender() = 0;

    /// Implement this method with logic to process data inside the stage
    virtual void Action(const uint start_index, const uint end_index) = 0;

  public:

    PipelineStage(uint pool_size, uint number_of_vehicles);

    virtual ~PipelineStage();

    void Start();

    void Stop();

  };

}
