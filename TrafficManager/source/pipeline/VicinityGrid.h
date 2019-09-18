#pragma once

#include <cmath>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "carla/client/Actor.h"
#include "carla/geom/Location.h"

namespace traffic_manager {

  class VicinityGrid {
  private:

    std::shared_timed_mutex modification_mutex;

    std::unordered_map<
        std::string, std::unordered_set<uint>
        > grid_to_actor_id;

    std::unordered_map<uint, std::string> actor_to_grid_id;

    std::string MakeKey(std::pair<int, int> grid_ids);

  public:

    VicinityGrid();
    ~VicinityGrid();

    std::unordered_set<uint> GetActors(carla::SharedPtr<carla::client::Actor> actor);

    std::pair<int, int> UpdateGrid(carla::SharedPtr<carla::client::Actor> actor);

    void EraseActor(uint actor_id);

  };

}
