// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/Episode.h"

#include "carla/client/detail/Client.h"

#include <boost/atomic.hpp>

#include <exception>

namespace carla {
namespace client {
namespace detail {

  EpisodeImpl::EpisodeImpl(SharedPtr<PersistentState> state)
    : _state(std::move(state)),
      _episode_id(_state->GetCurrentEpisodeId()) {}

  void EpisodeImpl::ClearState() {
    boost::atomic_store_explicit(&_state, {nullptr}, boost::memory_order_relaxed);
  }

  PersistentState &EpisodeImpl::GetPersistentStateWithChecks() const {
    auto state = boost::atomic_load_explicit(&_state, boost::memory_order_relaxed);
    if (state == nullptr) {
      throw std::runtime_error(
          "trying to operate on a destroyed actor; an actor's function "
          "was called, but the actor is already destroyed.");
    }
    if (_episode_id != _state->GetCurrentEpisodeId()) {
      throw std::runtime_error(
          "trying to access an expired episode; a new episode was started "
          "in the simulation but an object tried accessing the old one.");
    }
    return *_state;
  }

} // namespace detail
} // namespace client
} // namespace carla
