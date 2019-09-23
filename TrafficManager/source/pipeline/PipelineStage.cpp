#include "PipelineStage.h"

namespace traffic_manager {

  PipelineStage::PipelineStage(
      uint pool_size,
      uint number_of_vehicles)
    : pool_size(pool_size),
      number_of_vehicles(number_of_vehicles) {

    action_start_counter.store(0u);
    action_finished_counter.store(0u);
    run_stage.store(true);
    run_receiver.store(true);
    run_threads.store(false);
    run_sender.store(false);
  }

  PipelineStage::~PipelineStage() {}

  void PipelineStage::Start() {

    data_receiver = std::make_unique<std::thread>(&PipelineStage::ReceiverThreadManager, this);
    for (auto i = 0u; i < pool_size; ++i) {
      action_threads.push_back(
          std::make_unique<std::thread>(&PipelineStage::ActionThreadManager, this, i));
    }
    data_sender = std::make_unique<std::thread>(&PipelineStage::SenderThreadManager, this);

  }

  void PipelineStage::Stop() {
    run_stage.store(false);
    data_receiver->join();
    for (auto &action: action_threads) {
      action->join();
    }
    data_sender->join();
  }

  void PipelineStage::ReceiverThreadManager() {

    while (run_stage.load()) {
      std::unique_lock<std::mutex> lock(thread_coordination_mutex);
      // Wait for notification from sender thread
      // Break waiting if messenger is stopped
      while (!run_receiver.load() && run_stage.load()) {
        wake_receiver_notifier.wait_for(lock, 1ms, [=] {return run_receiver.load();});
      }
      run_receiver.store(false);

      // Receive data
      if (run_stage.load()) {
        DataReceiver();
      }

      // Wait for all worker threads to reach start condition
      while (action_start_counter.load() < pool_size && run_stage.load()) {
        wake_receiver_notifier.wait_for(lock, 1ms, [=] {return action_start_counter.load() == pool_size;});
      }

      // Start all workers
      run_threads.store(true);
      action_start_counter.store(0u);
      action_finished_counter.store(0u);

      wake_action_notifier.notify_all();
      lock.unlock();
    }
  }

  void PipelineStage::ActionThreadManager(const uint thread_id) {

    uint array_size = number_of_vehicles;
    uint load_per_thread = static_cast<uint>(std::floor(array_size / pool_size));

    while (run_stage.load()) {

      std::unique_lock<std::mutex> lock(thread_coordination_mutex);

      // Notify receiver after all worker threads reach start condition
      ++action_start_counter;
      if (action_start_counter == pool_size) {
        wake_receiver_notifier.notify_one();
      }

      // Wait for receiver to send start command
      while (!run_threads.load() && run_stage.load()) {
        wake_action_notifier.wait_for(lock, 1ms, [=] {return run_threads.load();});
      }
      lock.unlock();

      uint array_start_index = thread_id * load_per_thread;

      uint array_end_index;
      if (thread_id == pool_size - 1) {
        array_end_index = array_size - 1;
      } else {
        array_end_index = (thread_id + 1) * load_per_thread - 1;
      }

      // Run action
      Action(array_start_index, array_end_index);
      ++action_finished_counter;

      // Wait for all workers to finish
      while (action_finished_counter.load() < pool_size && run_stage.load()) {
        std::this_thread::sleep_for(1us);
      }

      if (run_threads.load()) {
        run_threads.store(false);
      }

      // Arbitrarily choosing last thread to notify sender
      // After all worker threads are done processing
      if (thread_id == pool_size - 1) {
        run_sender.store(true);
        wake_sender_notifier.notify_one();
      }
    }
  }

  void PipelineStage::SenderThreadManager() {

    while (run_stage.load()) {
      std::unique_lock<std::mutex> lock(thread_coordination_mutex);

      // Wait for notification from worker threads
      while (!run_sender.load() && run_stage.load()) {
        wake_sender_notifier.wait_for(lock, 1ms, [=] {return run_sender.load();});
      }
      run_sender.store(false);

      // Send data
      if (run_stage.load()) {
        DataSender();
      }

      // Notify receiver
      run_receiver.store(true);
      wake_receiver_notifier.notify_one();
      lock.unlock();
    }
  }

}
