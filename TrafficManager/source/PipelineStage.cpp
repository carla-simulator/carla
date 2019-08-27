// Definition file for members of class PipelineStage

#include "PipelineStage.h"

namespace traffic_manager {

  PipelineStage::PipelineStage(
      int pool_size,
      PipelineCallable &thread_callable)
    : pool_size(pool_size),
      thread_callable(thread_callable) {}

  PipelineStage::~PipelineStage() {}

  void PipelineStage::runThreads() {
    for (int range = pool_size; range > 0; range--) {
      threads.push_back(
          std::thread(
          &PipelineCallable::run,
          &thread_callable));
    }
  }

  void PipelineStage::start() {
    this->runThreads();
  }

  void PipelineStage::stop() {
    thread_callable.stop();
  }

}
