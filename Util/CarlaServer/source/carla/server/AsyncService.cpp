// CARLA, Copyright (C) 2017 Computer Vision Center (CVC) Project Settings.

#include "carla/Logging.h"
#include "carla/server/AsyncService.h"

namespace carla {
namespace server {

  AsyncService::AsyncService() {
    _thread = std::thread([this] {
      while (!_queue.done()) {
        job_type job;
        if (_queue.WaitAndPop(job)) {
          job();
        }
      }
    });
  }

  AsyncService::~AsyncService() {
    _queue.set_done();
    if (_thread.joinable()) {
      _thread.join();
    }
  }

} // namespace server
} // namespace carla
