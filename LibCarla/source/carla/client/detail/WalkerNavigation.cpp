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

    // query the navigation to find a path of points
    // std::vector<carla::geom::Location> Path;
    // if (!_nav.GetPath(From, To, nullptr, Path)) {
    //   logging::log("NAV: Path not found");
    // }
    // return Path;

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
        // set current height of the walker, ignoring height from recast
        trans.location.z = state.GetActorState(handle.walker).transform.location.z;
        float speed = _nav.GetWalkerSpeed(handle.walker);

        commands.emplace_back(Cmd::ApplyTransform{ handle.walker, trans });
        commands.emplace_back(Cmd::ApplyWalkerControl{ handle.walker, rpc::WalkerControl(trans.GetForwardVector(), speed , false)});
      }
    }

    _client.ApplyBatch(std::move(commands), false);
  }

} // namespace detail
} // namespace client
} // namespace carla
