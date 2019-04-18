// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MoveHandler.h"
#include "carla/NonCopyable.h"
#include "carla/ThreadGroup.h"
#include "carla/Time.h"

#include <boost/asio/io_service.hpp>

#include <future>
#include <thread>
#include <type_traits>

namespace carla {

  /// A thread pool based on Boost.Asio's io service.
  class ThreadPool : private NonCopyable {
  public:

    ThreadPool() : _work_to_do(_io_service) {}

    /// Stops the ThreadPool and joins all its threads.
    ~ThreadPool() {
      Stop();
    }

    /// Return the underlying io service.
    auto &service() {
      return _io_service;
    }

    /// Post a task to the pool.
    template <typename FunctorT, typename ResultT = typename std::result_of<FunctorT()>::type>
    std::future<ResultT> Post(FunctorT &&functor) {
      auto task = std::packaged_task<ResultT()>(std::forward<FunctorT>(functor));
      auto future = task.get_future();
      _io_service.post(carla::MoveHandler(task));
      return future;
    }

    /// Launch threads to run tasks asynchronously. Launch specific number of
    /// threads if @a worker_threads is provided, otherwise use all available
    /// hardware concurrency.
    void AsyncRun(size_t worker_threads) {
      _workers.CreateThreads(worker_threads, [this]() { Run(); });
    }

    /// @copydoc AsyncRun(size_t)
    void AsyncRun() {
      AsyncRun(std::thread::hardware_concurrency());
    }

    /// Run tasks in this thread.
    ///
    /// @warning This function blocks until the ThreadPool has been stopped.
    void Run() {
      _io_service.run();
    }

    /// Run tasks in this thread for an specific @a duration.
    ///
    /// @warning This function blocks until the ThreadPool has been stopped, or
    /// until the specified time duration has elapsed.
    void RunFor(time_duration duration) {
      _io_service.run_for(duration.to_chrono());
    }

    /// Stop the ThreadPool and join all its threads.
    void Stop() {
      _io_service.stop();
      _workers.JoinAll();
    }

  private:

    boost::asio::io_service _io_service;

    boost::asio::io_service::work _work_to_do;

    ThreadGroup _workers;
  };

} // namespace carla
