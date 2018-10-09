// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Memory.h"
#include "carla/client/detail/PersistentState.h"

namespace carla {
namespace client {
namespace detail {

  class EpisodeImpl {
  public:

    PersistentState &operator*() const {
      return GetPersistentStateWithChecks();
    }

    PersistentState *operator->() const {
      return &GetPersistentStateWithChecks();
    }

  protected:

    EpisodeImpl(SharedPtr<PersistentState> state);

    void ClearState();

  private:

    PersistentState &GetPersistentStateWithChecks() const;

    SharedPtr<PersistentState> _state;

    size_t _episode_id;
  };

  class Episode : private EpisodeImpl {
  public:

    using EpisodeImpl::operator*;
    using EpisodeImpl::operator->;

  private:

    friend PersistentState;

    using EpisodeImpl::EpisodeImpl;
  };

} // namespace detail
} // namespace client
} // namespace carla
