// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MoveHandler.h"
#include "carla/NonCopyable.h"
#include "carla/ThreadGroup.h"
#include "carla/Time.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>

#include <future>
#include <thread>
#include <optional>
#include <type_traits>

namespace carla {

  /// A thread pool based on Boost.Asio's io context.
  class ThreadPool :
      private NonCopyable
  {
  public:

    ThreadPool() :
      _io_context(),
      _work_guard(),
      _workers()
    {
    }

    /// Stops the ThreadPool and joins all its threads.
    ~ThreadPool()
    {
      Stop();
    }

    /// Return the underlying io_context.
    auto &io_context()
    {
      return _io_context;
    }

    /// Post a task to the pool.
    template <typename FunctorT>
    auto Post(FunctorT &&functor)
    {
      using ResultT = std::invoke_result_t<FunctorT()>;
      auto task = std::packaged_task<ResultT()>(std::forward<FunctorT>(functor));
      auto future = task.get_future();
      boost::asio::post(_io_context, carla::MoveHandler(task));
      return future;
    }

    /// Launch threads to run tasks asynchronously. Launch specific number of
    /// threads if @a worker_threads is provided, otherwise use all available
    /// hardware concurrency.
    void AsyncRun(size_t worker_threads)
    {
      _work_guard.emplace(
        boost::asio::make_work_guard(
          _io_context.get_executor()));
      _workers.CreateThreads(worker_threads, [this]() { Run(); });
    }

    /// @copydoc AsyncRun(size_t)
    void AsyncRun()
    {
      AsyncRun(std::thread::hardware_concurrency());
    }

    /// Run tasks in this thread.
    ///
    /// @warning This function blocks until the ThreadPool has been stopped.
    void Run()
    {
      _io_context.run();
    }

    /// Run tasks in this thread for an specific @a duration.
    ///
    /// @warning This function blocks until the ThreadPool has been stopped, or
    /// until the specified time duration has elapsed.
    void RunFor(time_duration duration)
    {
      _io_context.run_for(duration.to_chrono());
    }

    /// Stop the ThreadPool and join all its threads.
    void Stop() {
      if (_work_guard)
        _work_guard.reset();
      _io_context.stop();
      _workers.JoinAll();
    }

  private:

    using work_guard_type =
      std::optional<
        boost::asio::executor_work_guard<
          boost::asio::io_context::executor_type>>;

    boost::asio::io_context _io_context;
    work_guard_type _work_guard;
    ThreadGroup _workers;

  };

} // namespace carla
