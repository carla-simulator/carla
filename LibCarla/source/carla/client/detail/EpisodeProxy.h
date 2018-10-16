// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"

namespace carla {
namespace client {
namespace detail {

  class Simulator;

  /// Private implementation of a EpisodeProxy.
  class EpisodeProxyImpl {
  public:

    Simulator &operator*() const {
      return GetSimulatorWithChecks();
    }

    Simulator *operator->() const {
      return &GetSimulatorWithChecks();
    }

  protected:

    EpisodeProxyImpl(SharedPtr<Simulator> simulator);

    void ClearState();

  private:

    Simulator &GetSimulatorWithChecks() const;

    SharedPtr<Simulator> _simulator;

    size_t _episode_id;
  };

  /// Provides access to the Simulator during a given episode. After the episode
  /// is ended any access to the simulator throws an std::runtime_error.
  class EpisodeProxy : private EpisodeProxyImpl {
  public:

    using EpisodeProxyImpl::operator*;
    using EpisodeProxyImpl::operator->;

  private:

    friend class Simulator;

    using EpisodeProxyImpl::EpisodeProxyImpl;
  };

} // namespace detail
} // namespace client
} // namespace carla
