
#pragma once

#include <map>
#include <cmath>
#include <memory>
#include <string>

#include "carla/Memory.h"
#include "carla/client/Waypoint.h"
#include "carla/geom/Location.h"

#include "SimpleWaypoint.hpp"


namespace traffic_manager {

    typedef std::vector<
        std::pair<
            carla::SharedPtr<carla::client::Waypoint>,
            carla::SharedPtr<carla::client::Waypoint>
        >
    > TopologyList;
    typedef std::vector<std::shared_ptr<SimpleWaypoint>> NodeList;

    class InMemoryMap {
        /*
        This class constructs a descretised local map cache.
        Instantiate the class with map toplogy from the simulator
        and run setUp() to construct the local map.
        */

        private:

        TopologyList topology;
        std::vector<std::shared_ptr<SimpleWaypoint>> dense_topology;
        NodeList entry_node_list;
        NodeList exit_node_list;

        public:

        InMemoryMap(TopologyList topology);
        ~InMemoryMap();

        /* Constructs the local map with a resolution of sampling_resolution. */
        void setUp(int sampling_resolution);

        /* Returns the closest waypoint to a given location on the map. */
        std::shared_ptr<SimpleWaypoint> getWaypoint(carla::geom::Location location);

        /* Returns the full list of descrete samples of the map in local cache. */
        std::vector<std::shared_ptr<SimpleWaypoint>> get_dense_topology();
    };

}
