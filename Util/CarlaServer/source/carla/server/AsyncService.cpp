// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

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
