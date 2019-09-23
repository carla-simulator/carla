#include "VicinityGrid.h"

namespace traffic_manager {

  VicinityGrid::VicinityGrid() {}

  VicinityGrid::~VicinityGrid() {}

  std::string VicinityGrid::MakeKey(std::pair<int, int> grid_ids) {
    return std::to_string(grid_ids.first) + std::to_string(grid_ids.second);
  }

  std::pair<int, int> VicinityGrid::UpdateGrid(carla::SharedPtr<carla::client::Actor> actor) {

    auto actor_id = actor->GetId();
    auto location = actor->GetLocation();
    int first = static_cast<int>(std::floor(location.x / 10.0f));
    int second = static_cast<int>(std::floor(location.y / 10.0f));

    auto new_grid_id = MakeKey({first, second});

    // If actor exists in the grid
    if (actor_to_grid_id.find(actor_id) != actor_to_grid_id.end()) {

      auto old_grid_id = actor_to_grid_id.at(actor_id);
      // If actor has moved into a new road/section/lane
      if (old_grid_id != new_grid_id) {
        std::unique_lock<std::shared_timed_mutex> lock(modification_mutex);
        // Remove actor from old grid position and update new position
        grid_to_actor_id.at(old_grid_id).erase(actor_id);
        actor_to_grid_id.insert(std::pair<carla::ActorId, std::string>(actor_id, new_grid_id));
        if (grid_to_actor_id.find(new_grid_id) != grid_to_actor_id.end()) {
          grid_to_actor_id.at(new_grid_id).insert(actor_id);
        } else {
          grid_to_actor_id.insert(std::pair<std::string, std::unordered_set<carla::ActorId>>(new_grid_id,
              {actor_id}));
        }
      }
    }
    // If actor is new, then add entries to map
    else {
      std::unique_lock<std::shared_timed_mutex> lock(modification_mutex);
      actor_to_grid_id.insert(std::pair<carla::ActorId, std::string>(actor_id, new_grid_id));
      grid_to_actor_id.insert(std::pair<std::string, std::unordered_set<carla::ActorId>>(new_grid_id,
          {actor_id}));
    }

    // Return updated grid position
    return {first, second};
  }

  std::unordered_set<carla::ActorId> VicinityGrid::GetActors(carla::SharedPtr<carla::client::Actor> actor) {

    auto grid_ids = UpdateGrid(actor);

    std::shared_lock<std::shared_timed_mutex> lock(modification_mutex);
    std::unordered_set<carla::ActorId> actors;
    // Search all surrounding grids and find any vehicles in them
    for (int i = -1; i <= 1; ++i) {
      for (int j = -1; j <= 1; ++j) {

        auto grid_key = MakeKey({grid_ids.first + i, grid_ids.second + j});
        if (grid_to_actor_id.find(grid_key) != grid_to_actor_id.end()) {
          for (auto actor: grid_to_actor_id.at(grid_key)) {
            actors.insert(actor);
          }
        }
      }
    }

    return actors;
  }

  void VicinityGrid::EraseActor(carla::ActorId actor_id) {
    auto grid_key = actor_to_grid_id.at(actor_id);
    actor_to_grid_id.erase(actor_id);
    grid_to_actor_id.at(grid_key).erase(actor_id);
  }

}
