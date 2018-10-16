// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/EpisodeProxy.h"

#include "carla/client/detail/Simulator.h"

#include <boost/atomic.hpp>

#include <exception>

namespace carla {
namespace client {
namespace detail {

  EpisodeProxyImpl::EpisodeProxyImpl(SharedPtr<Simulator> simulator)
    : _simulator(std::move(simulator)),
      _episode_id(_simulator->GetCurrentEpisodeId()) {}

  void EpisodeProxyImpl::ClearState() {
    boost::atomic_store_explicit(&_simulator, {nullptr}, boost::memory_order_relaxed);
  }

  Simulator &EpisodeProxyImpl::GetSimulatorWithChecks() const {
    auto state = boost::atomic_load_explicit(&_simulator, boost::memory_order_relaxed);
    if (state == nullptr) {
      throw std::runtime_error(
          "trying to operate on a destroyed actor; an actor's function "
          "was called, but the actor is already destroyed.");
    }
    if (_episode_id != _simulator->GetCurrentEpisodeId()) {
      throw std::runtime_error(
          "trying to access an expired episode; a new episode was started "
          "in the simulation but an object tried accessing the old one.");
    }
    return *_simulator;
  }

} // namespace detail
} // namespace client
} // namespace carla
