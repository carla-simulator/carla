#pragma once

#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <mutex>
#include <shared_mutex>
#include <string>

#include "carla/geom/Location.h"
#include "carla/client/Actor.h"

namespace traffic_manager {

  class VicinityGrid {
    private:

    std::shared_timed_mutex modification_mutex;

    std::unordered_map<
      std::string, std::unordered_set<uint>
    > grid_to_actor_id;

    std::unordered_map<uint, std::string> actor_to_grid_id;

    std::string MakeKey(std::pair<int, int> gird_numbers);

    public:

    VicinityGrid();
    ~VicinityGrid();

    std::unordered_set<uint> GetActors(carla::SharedPtr<carla::client::Actor> actor);

  };
}
