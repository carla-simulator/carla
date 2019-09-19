#pragma once

#include <array>
#include <vector>

#include "carla/client/Map.h"
#include "carla/client/Waypoint.h"
#include "carla/Memory.h"

namespace traffic_manager {

<<<<<<< HEAD
/// This class is responsible for retrieving data from server
=======
  /// This class is responsible for retrieving data from server
>>>>>>> e2c8e19611819ecbb7026355674ba94b985ad488
  class CarlaDataAccessLayer {

    private:

<<<<<<< HEAD
=======
      /// Pointer to carla's map object
>>>>>>> e2c8e19611819ecbb7026355674ba94b985ad488
      carla::SharedPtr<carla::client::Map> world_map;

    public:

      CarlaDataAccessLayer(carla::SharedPtr<carla::client::Map> world_map);
      
      ~CarlaDataAccessLayer();

      /// Retrieves list of topology segments from the simulator
      using WaypointPtr = carla::SharedPtr<carla::client::Waypoint>;
      std::vector<std::pair<WaypointPtr, WaypointPtr>> GetTopology() const;

  };

}
