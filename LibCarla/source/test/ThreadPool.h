// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <carla/MoveHandler.h>
#include <carla/streaming/detail/AsioThreadPool.h>

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

    template <typename ResultT, typename FunctorT>
    std::future<ResultT> Post(FunctorT &&functor) {
      auto task = std::packaged_task<ResultT()>(std::forward<FunctorT>(functor));
      auto future = task.get_future();
      _pool.service().post(carla::MoveHandler(task));
      return future;
    }

  private:

    carla::streaming::detail::AsioThreadPool _pool;
  };

} // namespace util
