// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <future>
#include <type_traits>

#include "carla/NonCopyable.h"
#include "carla/server/ThreadSafeQueue.h"

namespace carla {
namespace server {

  /// Asynchronous service. Posted tasks are executed in a single separate
  /// thread in order of submission.
  class AsyncService : private NonCopyable {
  private:

    using job_type = std::function<void()>;

  public:

    AsyncService();

    ~AsyncService();

    bool done() const {
      return _queue.done();
    }

    /// Post a task to be executed by the asynchronous process. Its return value
    /// or exception thrown is stored in a shared state which can be accessed
    /// through the returned std::future object.
    template <typename F, typename R = std::result_of_t<F()>>
    std::future<R> Post(F task) {
      auto ptask = std::make_shared<std::packaged_task<R()>>(std::move(task));
      auto future = ptask->get_future();
      _queue.Push([ptask{std::move(ptask)}]() { (*ptask)(); });
      return future;
    }

  private:

    ThreadSafeQueue<job_type> _queue;

    std::thread _thread;
  };

} // namespace server
} // namespace carla
