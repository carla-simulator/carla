#pragma once

#include <cmath>
#include <vector>
#include <thread>
#include <memory>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>

#include "Messenger.h"

namespace traffic_manager {

  class PipelineStage {
    /// This class provides base functionality and template for
    /// Various stages of the pipeline

  private:

    const int pool_size;
    const int number_of_vehicles;

    std::shared_ptr<std::thread> data_receiver;
    std::shared_ptr<std::thread> data_sender;
    std::vector<std::shared_ptr<std::thread>> action_threads;

    std::atomic<int> action_counter;
    std::atomic<bool> run_stage;
    std::atomic<bool> run_threads;
    std::atomic<bool> run_receiver;
    std::atomic<bool> run_sender;

    std::mutex wait_for_action_mutex;
    std::mutex wait_receiver_mutex;
    std::mutex wait_sender_mutex;
    std::condition_variable wake_action_notifier;
    std::condition_variable wake_receiver_notifier;
    std::condition_variable wake_sender_notifier;

    void ReceiverThreadManager();

    void ActionThreadManager(int thread_id);

    void SenderThreadManager();

  protected:

    /// Implement this method with the logic to recieve data from
    /// Previous stage(s) and distribute to Action() threads
    virtual void DataReceiver()=0;

    /// Implement this method with logic to gather results from action
    /// Threads and send to next stage(s)
    virtual void DataSender()=0;

    /// Implement this method with logic to process data inside the stage
    virtual void Action(int array_index)=0;

  public:

    /// Pass the number of thread pool size.
    PipelineStage (
      int pool_size,
      int number_of_vehicles
    );

    virtual ~PipelineStage();

    /// This method starts the threads for the stage.
    void Start();

    /// This method stops all threads of the stage.
    void Stop();

  };

}
