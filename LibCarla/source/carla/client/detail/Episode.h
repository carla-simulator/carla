// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicSharedPtr.h"
#include "carla/NonCopyable.h"
#include "carla/client/detail/EpisodeState.h"
#include "carla/rpc/EpisodeInfo.h"

namespace carla {
namespace client {
namespace detail {

  class Client;

  /// Represents the episode running on the Simulator.
  class Episode
    : public std::enable_shared_from_this<Episode>,
      private NonCopyable {
  public:

    explicit Episode(Client &client);

    ~Episode();

    void Listen();

    auto GetId() const {
      return _description.id;
    }

    const std::string &GetMapName() const {
      return _description.map_name;
    }

    std::shared_ptr<const EpisodeState> GetState() const {
      auto state = _state.load();
      DEBUG_ASSERT(state != nullptr);
      return state;
    }

  private:

    Client &_client;

    const rpc::EpisodeInfo _description;

    AtomicSharedPtr<const EpisodeState> _state;
  };

} // namespace detail
} // namespace client
} // namespace carla
