// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <carla/streaming/detail/AsioThreadPool.h>

#include <functional>
#include <future>
#include <memory>
#include <thread>

namespace util {

  class ThreadPool {
  public:

    ThreadPool(size_t worker_threads = 0u) {
      _pool.AsyncRun(
          worker_threads > 0u ? worker_threads : std::thread::hardware_concurrency());
    }

    template <typename ResultT>
    std::future<ResultT> Post(std::function<ResultT()> functor) {
      auto task = std::make_shared<std::packaged_task<ResultT()>>(std::move(functor));
      auto future = task->get_future();
      _pool.service().post([t=std::move(task)]() mutable { (*t)(); });
      return future;
    }

  private:

    carla::streaming::detail::AsioThreadPool _pool;
  };

} // namespace util
