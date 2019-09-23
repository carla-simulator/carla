#pragma once

#include <cmath>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "carla/client/Actor.h"
#include "carla/geom/Location.h"
#include "carla/rpc/Actor.h"

namespace traffic_manager {

  /// This class maintains vehicle positions in grid segments.
  /// This is used in collision stage to filter vehicles.
  class VicinityGrid {

  private:

    /// Mutex to manage contention between modifiers and readers
    std::shared_timed_mutex modification_mutex;
    /// Map connecting grid id to set of actor ids
    using ActorIds = std::unordered_set<carla::ActorId>;
    std::unordered_map<std::string, ActorIds> grid_to_actor_id;
    /// Map connecting actor id to grid key
    std::unordered_map<carla::ActorId, std::string> actor_to_grid_id;

    /// Key generator for given grid
    std::string MakeKey(std::pair<int, int> grid_ids);

  public:

    VicinityGrid();
    ~VicinityGrid();

    /// Returns a set of actors in the vicinity of a given actor
    std::unordered_set<carla::ActorId> GetActors(carla::SharedPtr<carla::client::Actor> actor);

    /// Updates the grid position of the given actor and returns new grid id
    std::pair<int, int> UpdateGrid(carla::SharedPtr<carla::client::Actor> actor);

    /// Removes actor
    void EraseActor(carla::ActorId actor_id);

  };

}
