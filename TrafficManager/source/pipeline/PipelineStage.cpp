#include "PipelineStage.h"

namespace traffic_manager {

  PipelineStage::PipelineStage (
    int pool_size,
    int number_of_vehicles
  ): pool_size(pool_size), number_of_vehicles(number_of_vehicles) 
  {

    action_counter = 0;
    run_stage = true;
    run_receiver = true;
    run_threads = false;
    run_sender = false;
  }

  PipelineStage::~PipelineStage() {}

  void PipelineStage::Start() {
    data_receiver = std::make_shared<std::thread>(&PipelineStage::ReceiverThreadManager, this);
    for (int i=0; i<pool_size; i++) {
      action_threads.push_back(
        std::make_shared<std::thread>(&PipelineStage::ActionThreadManager, this, i)
      );
    }
    data_sender = std::make_shared<std::thread>(&PipelineStage::SenderThreadManager, this);
  }

  void PipelineStage::Stop() {
    run_stage.store(false);
  }

  void PipelineStage::ReceiverThreadManager() {
    while (run_stage.load()) {
      std::unique_lock<std::mutex> lock(wait_receiver_mutex);
      if (!run_receiver.load()) {
        wake_receiver_notifier.wait(lock, [=] {return run_receiver.load();});
      }

      this->DataReceiver();

      run_threads.store(true);
      wake_action_notifier.notify_all();
      run_receiver.store(false);
    }
  }

  void PipelineStage::ActionThreadManager(int thread_id) {
    int array_size = number_of_vehicles;
    int load_per_thread = std::floor(array_size/pool_size);

    while (run_stage.load()) {
      std::unique_lock<std::mutex> lock(wait_for_action_mutex);
      wake_action_notifier.wait(lock, [=] {return run_stage.load();});
      lock.unlock();

      int array_start_index = thread_id*load_per_thread;
      int array_end_index = thread_id == pool_size-1 ? array_size-1 : (thread_id+1)*load_per_thread-1;

      for (int i = array_start_index; i < array_end_index; i++) {
        this->Action(i);
      }

      action_counter++;

      if (run_threads.load()) {
        run_threads.store(false);
      }

      if (thread_id == pool_size-1) {
        while (action_counter.load() < pool_size);
        action_counter.store(0);
        run_sender.store(true);
        wake_sender_notifier.notify_one();
      }
    }
  }

  void PipelineStage::SenderThreadManager() {
    while (run_stage.load()) {
      std::unique_lock<std::mutex> lock(wait_sender_mutex);
      if (!run_sender.load()) {
        wake_sender_notifier.wait(lock, [=] {return run_sender.load();});
      }

      this->DataSender();

      run_receiver.store(true);
      wake_receiver_notifier.notify_one();
      run_sender.store(false);
    }
  }

}
