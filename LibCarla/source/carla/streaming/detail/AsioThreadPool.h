// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/ThreadGroup.h"
#include "carla/profiler/LifetimeProfiled.h"

#include <boost/asio/io_service.hpp>

namespace carla {
namespace streaming {
namespace detail {

  class AsioThreadPool
    : private profiler::LifetimeProfiled,
      private NonCopyable {
  public:

    AsioThreadPool()
      : LIBCARLA_INITIALIZE_LIFETIME_PROFILER("AsioThreadPool"),
        _work_to_do(_io_service) {}

    ~AsioThreadPool() {
      Stop();
    }

    auto &service() {
      return _io_service;
    }

    void Run() {
      _io_service.run();
    }

    void AsyncRun(size_t worker_threads) {
      _workers.CreateThreads(worker_threads, [this]() { Run(); });
    }

    void Stop() {
      _io_service.stop();
      _workers.JoinAll();
    }

  private:

    boost::asio::io_service _io_service;

    boost::asio::io_service::work _work_to_do;

    ThreadGroup _workers;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
