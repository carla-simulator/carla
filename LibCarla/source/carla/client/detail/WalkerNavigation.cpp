// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/WalkerNavigation.h"

#include "carla/client/detail/Client.h"
#include "carla/client/detail/EpisodeState.h"
#include "carla/rpc/Command.h"

namespace carla {
namespace client {
namespace detail {

  static geom::Transform TickWalker(
      const float delta_seconds,
      geom::Transform transform) {
    transform.location += delta_seconds * 2.778f * transform.GetForwardVector();
    return transform;
  }

  WalkerNavigation::WalkerNavigation(Client &client) : _client(client) {
    // Here call the server to retrieve the navmesh data.
  }

  void WalkerNavigation::Tick(const EpisodeState &state) const {
    auto walkers = _walkers.Load();
    if (walkers->empty()) {
      return;
    }

    using Cmd = rpc::Command;
    std::vector<Cmd> commands;
    commands.reserve(walkers->size());

    for (auto handle : *walkers) {
      commands.emplace_back(
          Cmd::ApplyTransform{
              handle.walker,
              TickWalker(
                  static_cast<float>(state.GetTimestamp().delta_seconds),
                  state.GetActorState(handle.walker).transform)});
    }

    _client.ApplyBatch(std::move(commands), false);
  }

} // namespace detail
} // namespace client
} // namespace carla
