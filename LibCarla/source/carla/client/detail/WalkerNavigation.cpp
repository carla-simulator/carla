// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/WalkerNavigation.h"

#include "carla/client/detail/Client.h"
#include "carla/client/detail/Episode.h"
#include "carla/client/detail/EpisodeState.h"
#include "carla/client/detail/Simulator.h"
#include "carla/nav/Navigation.h"
#include "carla/rpc/Command.h"
#include "carla/rpc/DebugShape.h"
#include "carla/rpc/WalkerControl.h"

#include <sstream>

namespace carla {
namespace client {
namespace detail {

  WalkerNavigation::WalkerNavigation(std::weak_ptr<Simulator> simulator) : _simulator(simulator), _next_check_index(0) {
    _nav.SetSimulator(simulator);
    // Here call the server to retrieve the navmesh data.
    auto files = _simulator.lock()->GetRequiredFiles("Nav");
    if (!files.empty()) {
      _nav.Load(_simulator.lock()->GetCacheFile(files[0], true));
    }
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
    UpdateVehiclesInCrowd(episode, false);

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
    _simulator.lock()->ApplyBatchSync(std::move(commands), false);

    // check if any agent has been killed
    bool alive;
    for (auto handle : *walkers) {
      // get the agent state
      if (_nav.IsWalkerAlive(handle.walker, alive)) {
        if (!alive) {
          _simulator.lock()->SetActorCollisions(handle.walker, true);
          _simulator.lock()->SetActorDead(handle.walker);
          // remove from the crowd
          _nav.RemoveAgent(handle.walker);
          // destroy the controller
          _simulator.lock()->DestroyActor(handle.controller);
          // unregister from list
          UnregisterWalker(handle.walker, handle.controller);
        }
      }
    }
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
      _simulator.lock()->DestroyActor(walkers[_next_check_index].controller);
      // unregister from list
      UnregisterWalker(walkers[_next_check_index].walker, walkers[_next_check_index].controller);
    }

    ++_next_check_index;

  }

  // add/update/delete all vehicles in crowd
  void WalkerNavigation::UpdateVehiclesInCrowd(std::shared_ptr<Episode> episode, bool show_debug) {
    std::vector<carla::nav::VehicleCollisionInfo> vehicles;

    // get current state
    std::shared_ptr<const EpisodeState> state = episode->GetState();

    // get all vehicles from episode
    for (auto &&actor : episode->GetActors()) {
      // only vehicles
      if (actor.description.id.rfind("vehicle.", 0) == 0) {
        // get the snapshot
        ActorSnapshot snapshot = state->GetActorSnapshot(actor.id);
        // add to the vector
        vehicles.emplace_back(carla::nav::VehicleCollisionInfo{actor.id, snapshot.transform, actor.bounding_box});
      }
    }

    // update the vehicles found
    _nav.UpdateVehicles(vehicles);

    // optional debug info
    if (show_debug) {
      if (_nav.GetCrowd() == nullptr) return;

      // draw bounding boxes for debug
      for (int i = 0; i < _nav.GetCrowd()->getAgentCount(); ++i) {
        // get the agent
        const dtCrowdAgent *agent = _nav.GetCrowd()->getAgent(i);
        if (agent && agent->params.useObb) {
          // draw for debug
          carla::geom::Location p1, p2, p3, p4;
          p1.x = agent->params.obb[0];
          p1.z = agent->params.obb[1];
          p1.y = agent->params.obb[2];
          p2.x = agent->params.obb[3];
          p2.z = agent->params.obb[4];
          p2.y = agent->params.obb[5];
          p3.x = agent->params.obb[6];
          p3.z = agent->params.obb[7];
          p3.y = agent->params.obb[8];
          p4.x = agent->params.obb[9];
          p4.z = agent->params.obb[10];
          p4.y = agent->params.obb[11];
          carla::rpc::DebugShape line1;
          line1.life_time = 0.01f;
          line1.persistent_lines = false;
          // line 1
          line1.primitive = carla::rpc::DebugShape::Line {p1, p2, 0.2f};
          line1.color = { 0, 255, 0 };
          _simulator.lock()->DrawDebugShape(line1);
          // line 2
          line1.primitive = carla::rpc::DebugShape::Line {p2, p3, 0.2f};
          line1.color = { 255, 0, 0 };
          _simulator.lock()->DrawDebugShape(line1);
          // line 3
          line1.primitive = carla::rpc::DebugShape::Line {p3, p4, 0.2f};
          line1.color = { 0, 0, 255 };
          _simulator.lock()->DrawDebugShape(line1);
          // line 4
          line1.primitive = carla::rpc::DebugShape::Line {p4, p1, 0.2f};
          line1.color = { 255, 255, 0 };
          _simulator.lock()->DrawDebugShape(line1);
        }
      }

      // draw some text for debug
      for (int i = 0; i < _nav.GetCrowd()->getAgentCount(); ++i) {
        // get the agent
        const dtCrowdAgent *agent = _nav.GetCrowd()->getAgent(i);
        if (agent) {
          // draw for debug
          carla::geom::Location p1(agent->npos[0], agent->npos[2], agent->npos[1] + 1);
          if (agent->params.userData) {
            std::ostringstream out;
            out << *(reinterpret_cast<const float *>(agent->params.userData));
            carla::rpc::DebugShape text;
            text.life_time = 0.01f;
            text.persistent_lines = false;
            text.primitive = carla::rpc::DebugShape::String {p1, out.str(), false};
            text.color = { 0, 255, 0 };
            _simulator.lock()->DrawDebugShape(text);
          }
        }
      }
    }
  }

} // namespace detail
} // namespace client
} // namespace carla
