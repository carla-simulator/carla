// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicList.h"
#include "carla/nav/Navigation.h"
#include "carla/NonCopyable.h"
#include "carla/client/Timestamp.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/rpc/ActorId.h"

#include <memory>

namespace carla {
namespace client {
namespace detail {

  class Client;
  class Episode;
  class EpisodeState;

  class WalkerNavigation
    : public std::enable_shared_from_this<WalkerNavigation>,
    private NonCopyable {
  public:

    explicit WalkerNavigation(Client & client);

    void RegisterWalker(ActorId walker_id, ActorId controller_id) {
      // add to list
      _walkers.Push(WalkerHandle { walker_id, controller_id });
    }

    void UnregisterWalker(ActorId walker_id, ActorId controller_id) {
      // remove from list
      auto list = _walkers.Load();
      unsigned int i = 0;
      while (i < list->size()) {
        if ((*list)[i].walker == walker_id &&
        (*list)[i].controller == controller_id) {
          _walkers.DeleteByIndex(i);
          break;
        }
        ++i;
      }
    }

    void RemoveWalker(ActorId walker_id) {
      // remove the walker in the crowd
      _nav.RemoveAgent(walker_id);
    }

    void AddWalker(ActorId walker_id, carla::geom::Location location) {
      // create the walker in the crowd (to manage its movement in Detour)
      _nav.AddWalker(walker_id, location);
    }

    void Tick(std::shared_ptr<Episode> episode);

    // Get Random location in nav mesh
    boost::optional<geom::Location> GetRandomLocation() {
      geom::Location random_location(0, 0, 0);
      if (_nav.GetRandomLocation(random_location))
        return boost::optional<geom::Location>(random_location);
      else
        return {};
    }

    // set a new target point to go
    bool SetWalkerTarget(ActorId id, const carla::geom::Location to) {
      return _nav.SetWalkerTarget(id, to);
    }

    // set new max speed
    bool SetWalkerMaxSpeed(ActorId id, float max_speed) {
      return _nav.SetWalkerMaxSpeed(id, max_speed);
    }

    // set percentage of pedestrians that can cross the road
    void SetPedestriansCrossFactor(float percentage) {
      _nav.SetPedestriansCrossFactor(percentage);
    }

    void SetPedestriansSeed(unsigned int seed) {
      _nav.SetSeed(seed);
    }

  private:

    Client &_client;

    unsigned long _next_check_index;

    carla::nav::Navigation _nav;

    struct WalkerHandle {
      ActorId walker;
      ActorId controller;
    };

    AtomicList<WalkerHandle> _walkers;

    /// check a few walkers and if they don't exist then remove from the crowd
    void CheckIfWalkerExist(std::vector<WalkerHandle> walkers, const EpisodeState &state);
    /// add/update/delete all vehicles in crowd
    void UpdateVehiclesInCrowd(std::shared_ptr<Episode> episode, bool show_debug = false);
  };

} // namespace detail
} // namespace client
} // namespace carla
