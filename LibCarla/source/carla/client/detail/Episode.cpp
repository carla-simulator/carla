// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/Episode.h"

#include "carla/client/detail/Client.h"

#include <exception>

namespace carla {
namespace client {
namespace detail {

  Episode::Episode(SharedPtr<PersistentState> state)
    : _state(std::move(state)),
      _episode_id(_state->GetCurrentEpisodeId()) {}

  PersistentState &Episode::GetPersistentStateWithChecks() const {
    DEBUG_ASSERT(_state != nullptr);
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
