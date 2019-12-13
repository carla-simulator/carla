// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cmath>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "carla/client/Actor.h"
#include "carla/geom/Location.h"
#include "carla/rpc/Actor.h"

#include "carla/trafficmanager/SimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

  namespace cc = carla::client;
  namespace cg = carla::geom;

  using ActorId = carla::ActorId;
  using Actor = carla::SharedPtr<cc::Actor>;
  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
  using Buffer = std::deque<SimpleWaypointPtr>;

  /// This class maintains vehicle positions in grid segments.
  /// This is used in the collision stage to filter vehicles.
  class VicinityGrid {

  private:

    /// Mutex to manage contention between modifiers and readers.
    std::shared_timed_mutex modification_mutex;
    /// Map connecting grid id to set of actor ids.
    std::unordered_map<std::string, std::unordered_set<ActorId>> grid_to_actor_id;
    /// Map connecting actor id to grid key.
    std::unordered_map<ActorId, std::string> actor_to_grid_id;

    /// Key generator for a given grid.
    std::string MakeKey(std::pair<int, int> grid_ids);

  public:

    VicinityGrid();
    ~VicinityGrid();

    /// Returns a set of actors in the vicinity of a given actor.
    std::unordered_set<ActorId> GetActors(Actor actor);

    /// Updates the grid position of the given actor and returns new grid id.
    std::pair<int, int> UpdateGrid(Actor actor);

    /// Removes actor.
    void EraseActor(ActorId actor_id);

  };

} // namespace traffic_manager
} // namespace carla
