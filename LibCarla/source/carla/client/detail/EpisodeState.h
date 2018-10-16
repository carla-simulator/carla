// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/rpc/EpisodeInfo.h"

namespace carla {
namespace client {
namespace detail {

  /// Represents an episode running on the Simulator.
  class EpisodeState : private MovableNonCopyable {
  public:

    EpisodeState() = default; /// @todo

    EpisodeState(rpc::EpisodeInfo description)
      : _description(std::move(description)) {}

    auto GetId() const {
      return _description.id;
    }

    const std::string &GetMapName() const {
      return _description.map_name;
    }

  private:

    rpc::EpisodeInfo _description;
  };

} // namespace detail
} // namespace client
} // namespace carla
