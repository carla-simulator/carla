// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/WalkerNavigation.h"

#include "carla/client/detail/Client.h"
#include "carla/client/detail/Episode.h"
#include "carla/client/detail/EpisodeState.h"
#include "carla/nav/Navigation.h"
#include "carla/rpc/Command.h"
#include "carla/rpc/DebugShape.h"
#include "carla/rpc/WalkerControl.h"

#include <sstream>

namespace carla {
namespace client {
namespace detail {

  WalkerNavigation::WalkerNavigation(Client &client) : _client(client), _next_check_index(0) {
    // Here call the server to retrieve the navmesh data.
    _nav.Load(_client.GetNavigationMesh());
  }

  void WalkerNavigation::Tick(std::shared_ptr<Episode> episode) {
    auto walkers = _walkers.Load();
    if (walkers->empty()) {
      return;
    }

    // get current state
    std::shared_ptr<const EpisodeState> state = episode->GetState();

    // purge all possible dead walkers
    CheckIfWalkerExist(*walkers, *state);

    // add/update/delete all vehicles in crowd
    UpdateVehiclesInCrowd(episode);

    // update crowd in navigation module
    _nav.UpdateCrowd(*state);

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

  void WalkerNavigation::CheckIfWalkerExist(std::vector<WalkerHandle> walkers, const EpisodeState &state) {

    // check with total
    if (_next_check_index >= walkers.size())
      _next_check_index = 0;

    // check the existence
    if (!state.ContainsActorSnapshot(walkers[_next_check_index].walker)) {
      // remove from the crowd
      _nav.RemoveAgent(walkers[_next_check_index].walker);
      // destroy the controller
      _client.DestroyActor(walkers[_next_check_index].controller);
      // unregister from list
      UnregisterWalker(walkers[_next_check_index].walker, walkers[_next_check_index].controller);
    }

    ++_next_check_index;

  }

  // add/update/delete all vehicles in crowd
  void WalkerNavigation::UpdateVehiclesInCrowd(std::shared_ptr<Episode> episode) {
    std::vector<carla::nav::VehicleCollisionInfo> vehicles;
    
    // get current state
    std::shared_ptr<const EpisodeState> state = episode->GetState();

    // get all vehicles from episode
    for (auto &&actor : episode->GetActors()) {
      // only vehicles
      if (actor.description.id.rfind("vehicle.", 0) == 0) {
        // logging::log("Nav: vehicle found: ", actor.description.id);
        // get the snapshot
        ActorSnapshot snapshot = state->GetActorSnapshot(actor.id);
        // add to the vector
        vehicles.emplace_back(carla::nav::VehicleCollisionInfo{actor.id, snapshot.transform, actor.bounding_box});
      }
    }

    // update the vehicles found
    _nav.UpdateVehicles(vehicles);
  }

} // namespace detail
} // namespace client
} // namespace carla
