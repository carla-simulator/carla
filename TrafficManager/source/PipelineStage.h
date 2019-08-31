#pragma once

#include <vector>
#include <thread>
#include <memory>
#include <shared_mutex>
#include <condition_variable>

#include "Messenger.h"
#include "StageEnum.h"

namespace traffic_manager {

  class PipelineStage {
    /// This class provides base functionality and template for
    /// Various stages of the pipeline

  private:

    std::unique_ptr<std::thread> data_reciever;
    std::unique_ptr<std::thread> data_sender;
    std::vector<std::unique_ptr<std::thread>> action_threads;

  protected:

    const int pool_size;
    bool run_threads =false;
    bool run_stage =true;

    std::shared_timed_mutex wait_for_start_mutex;
    std::condition_variable_any wake_up_notifier;

    /// Implement this method with the logic to recieve data from
    /// Previous stage(s) and distribute to Action() threads
    virtual void DataReciever()=0;

    /// Implement this method with logic to gather results from action
    /// Threads and send to next stage(s)
    virtual void DataSender()=0;

    /// Implement this method with logic to process data inside the stage
    virtual void Action(int thread_id)=0;

  public:

    /// Pass the number of thread pool size.
    PipelineStage (
      int pool_size
    );

    virtual ~PipelineStage();

    /// This method starts the threads for the stage.
    void Start();

    /// This method stops all threads of the stage.
    void Stop();

  };

}
