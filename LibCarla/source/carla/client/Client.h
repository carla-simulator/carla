// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/PythonUtil.h"
#include "carla/client/World.h"
#include "carla/client/detail/Simulator.h"

namespace carla {
namespace client {

  class Client {
  public:

    /// Construct a carla client.
    ///
    /// @param host IP address of the host machine running the simulator.
    /// @param port TCP port to connect with the simulator.
    /// @param worker_threads number of asynchronous threads to use, or 0 to use
    ///        all available hardware concurrency.
    explicit Client(
        const std::string &host,
        uint16_t port,
        size_t worker_threads = 0u);

    void SetTimeout(time_duration timeout) {
      _simulator->SetNetworkingTimeout(timeout);
    }

    std::string GetClientVersion() const {
      return _simulator->GetClientVersion();
    }

    std::string GetServerVersion() const {
      return _simulator->GetServerVersion();
    }

    bool Ping() const {
      return _simulator->Ping();
    }

    World GetWorld() const {
      return World{_simulator->GetCurrentEpisode()};
    }

  private:

    SharedPtr<detail::Simulator> _simulator;
  };

  inline Client::Client(
      const std::string &host,
      uint16_t port,
      size_t worker_threads)
    : _simulator(
        new detail::Simulator(host, port, worker_threads),
        PythonUtil::ReleaseGILDeleter()) {}

} // namespace client
} // namespace carla
