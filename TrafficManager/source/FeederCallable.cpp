// definition of FeederCallable class members

#include "FeederCallable.h"

namespace traffic_manager {

  const float UPDATE_FREQUENCY = 30;

  Feedercallable::Feedercallable(
      SyncQueue<PipelineMessage> *input_queue,
      SyncQueue<PipelineMessage> *output_queue,
      SharedData *shared_data)
    : PipelineCallable(input_queue, output_queue, shared_data) {}
  Feedercallable::~Feedercallable() {}

  PipelineMessage Feedercallable::action(PipelineMessage &message) {

    while (!exit_flag) {

      auto compute_time_start = std::chrono::system_clock::now();
      for (auto actor: shared_data->registered_actors) {
        if (
          actor != nullptr
          and
          actor->IsAlive()
        ) {
          message.setActor(actor);
          writeQueue(message);
        }
      }
      auto compute_time_end = std::chrono::system_clock::now();
      std::chrono::duration<double> compute_duration = compute_time_end - compute_time_start;

      double cpu_redundancy_time = (1.0/UPDATE_FREQUENCY) - compute_duration.count();
      int redundancy_microseconds = static_cast<int>(std::floor(cpu_redundancy_time * 1000000));
      redundancy_microseconds = redundancy_microseconds > 0 ? redundancy_microseconds: 0;
      std::this_thread::sleep_for(std::chrono::microseconds(redundancy_microseconds));
    }
    PipelineMessage empty_message;
    return empty_message;
  }
}
