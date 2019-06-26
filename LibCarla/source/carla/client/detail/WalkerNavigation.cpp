// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/WalkerNavigation.h"

#include "carla/client/detail/Client.h"
#include "carla/client/detail/EpisodeState.h"
#include "carla/rpc/Command.h"
#include "carla/rpc/WalkerControl.h"

namespace carla {
namespace client {
namespace detail {

  WalkerNavigation::WalkerNavigation(Client &client) : _client(client) {
    // Here call the server to retrieve the navmesh data.
    _nav.Load(_client.GetNavigationMesh());
  }

  void WalkerNavigation::Tick(const EpisodeState &state) {
    auto walkers = _walkers.Load();
    if (walkers->empty()) {
      return;
    }

    // update crowd in navigation module
    _nav.UpdateCrowd(state);

    carla::geom::Transform trans;
    using Cmd = rpc::Command;
    std::vector<Cmd> commands;
    commands.reserve(walkers->size());
    for (auto handle : *walkers) {
      // get the transform of the walker
      if (_nav.GetWalkerTransform(handle.walker, trans)) {
        float speed = _nav.GetWalkerSpeed(handle.walker);
        commands.emplace_back(Cmd::ApplyWalkerState{ handle.walker, trans, speed });
      }
    }

    _client.ApplyBatch(std::move(commands), false);
  }

} // namespace detail
} // namespace client
} // namespace carla
