
#pragma once

#include <memory.h>

#include "carla/Memory.h"
#include "carla/geom/Vector3D.h"
#include "carla/geom/Location.h"
#include "carla/client/Waypoint.h"

namespace traffic_manager {

    class SimpleWaypoint {
        /*
        This is a simple wrapper class on carla's waypoint object.
        The class is used to represent descrete samples of the world map.
        */

        private:

        carla::SharedPtr<carla::client::Waypoint> waypoint;
        std::vector<std::shared_ptr<SimpleWaypoint>> next_waypoints;

        public:

        SimpleWaypoint(carla::SharedPtr<carla::client::Waypoint> waypoint);
        ~SimpleWaypoint();

        /* Returns the location object for this waypoint. */
        carla::geom::Location getLocation();

        /* Returns the list of next waypoints. */
        std::vector<std::shared_ptr<SimpleWaypoint>> getNextWaypoint();

        /* Returns the vector along the waypoint's direction. */
        carla::geom::Vector3D getVector();

        /* Returns the location of the waypoint as a list of x,y,z float values. */
        std::vector<float> getXYZ();

        /* This method is used to set the next waypoints. */
        int setNextWaypoint(std::vector<std::shared_ptr<SimpleWaypoint>> next_waypoints);

        /* Calculates the distance from the object's waypoint to the passed location. */
        float distance(carla::geom::Location location);

        /* Returns true if the object's waypoint belongs to an intersection. */
        bool checkJunction();
    };

}
