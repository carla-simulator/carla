// Member definitions for class PipelineCallable

#include "PipelineCallable.h"

namespace traffic_manager {

  PipelineCallable::PipelineCallable(
      SyncQueue<PipelineMessage> *input_queue,
      SyncQueue<PipelineMessage> *output_queue,
      SharedData *shared_data)
    : input_queue(input_queue),
      output_queue(output_queue),
      shared_data(shared_data) {}
  PipelineCallable::~PipelineCallable() {}

  PipelineMessage PipelineCallable::readQueue() {
    return input_queue->pop();
  }

  void PipelineCallable::writeQueue(PipelineMessage message) {
    output_queue->push(message);
  }

  void PipelineCallable::run() {
    while (!exit_flag) {
      PipelineMessage in_message;
      if (input_queue != NULL) {
        in_message = readQueue();
      }
      auto out_message = action(in_message);
      if (output_queue != NULL) {
        writeQueue(out_message);
      }
    }
  }

  void PipelineCallable::stop() {
    exit_flag = true;
  }

}
