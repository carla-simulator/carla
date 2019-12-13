// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "VicinityGrid.h"

namespace carla {
namespace traffic_manager {

  static const float GRID_SIZE = 10.0f;

  VicinityGrid::VicinityGrid() {}

  VicinityGrid::~VicinityGrid() {}

  std::string VicinityGrid::MakeKey(std::pair<int, int> grid_ids) {
    return std::to_string(grid_ids.first) + std::to_string(grid_ids.second);
  }

  std::pair<int, int> VicinityGrid::UpdateGrid(Actor actor) {

    const ActorId actor_id = actor->GetId();
    const cg::Location location = actor->GetLocation();
    const int first = static_cast<int>(std::floor(location.x / GRID_SIZE));
    const int second = static_cast<int>(std::floor(location.y / GRID_SIZE));

    const std::string new_grid_id = MakeKey({first, second});

    // If the actor exists in the grid.
    if (actor_to_grid_id.find(actor_id) != actor_to_grid_id.end()) {

      const std::string old_grid_id = actor_to_grid_id.at(actor_id);

      // If the actor has moved into a new road/section/lane.
      if (old_grid_id != new_grid_id) {
        std::unique_lock<std::shared_timed_mutex> lock(modification_mutex);

        // Update the actor's grid id.
        actor_to_grid_id.at(actor_id) = new_grid_id;

        // Remove the actor from the old grid position and update the new position.
        grid_to_actor_id.at(old_grid_id).erase(actor_id);
        if (grid_to_actor_id.find(new_grid_id) != grid_to_actor_id.end()) {
          grid_to_actor_id.at(new_grid_id).insert(actor_id);
        } else {
          grid_to_actor_id.insert({new_grid_id, {actor_id}});
        }
      }
    }
    // If the actor is new, then add entries to map.
    else {

      std::unique_lock<std::shared_timed_mutex> lock(modification_mutex);
      actor_to_grid_id.insert({actor_id, new_grid_id});
      if (grid_to_actor_id.find(new_grid_id) != grid_to_actor_id.end()) {
        grid_to_actor_id.at(new_grid_id).insert(actor_id);
      } else {
        grid_to_actor_id.insert({new_grid_id, {actor_id}});
      }
    }

    // Return updated grid position.
    return {first, second};
  }

  std::unordered_set<carla::ActorId> VicinityGrid::GetActors(Actor actor) {

    const std::pair<int, int> grid_ids = UpdateGrid(actor);

    std::shared_lock<std::shared_timed_mutex> lock(modification_mutex);
    std::unordered_set<ActorId> actors;

    // Search all surrounding grids and find any vehicles in them.
    for (int i = -1; i <= 1; ++i) {
      for (int j = -1; j <= 1; ++j) {

        const std::string grid_key = MakeKey({grid_ids.first + i, grid_ids.second + j});
        if (grid_to_actor_id.find(grid_key) != grid_to_actor_id.end()) {
          const auto &grid_actor_set = grid_to_actor_id.at(grid_key);
          actors.insert(grid_actor_set.begin(), grid_actor_set.end());
        }
      }
    }

    return actors;
  }

  void VicinityGrid::EraseActor(ActorId actor_id) {
    const std::string grid_key = actor_to_grid_id.at(actor_id);
    actor_to_grid_id.erase(actor_id);
    grid_to_actor_id.at(grid_key).erase(actor_id);
  }

} // namespace traffic_manager
} // namespace carla
