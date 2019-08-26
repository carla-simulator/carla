#pragma once

#include <memory.h>

#include "carla/Memory.h"
#include "carla/geom/Vector3D.h"
#include "carla/geom/Location.h"
#include "carla/client/Waypoint.h"

namespace traffic_manager {

  class SimpleWaypoint {

    /// This is a simple wrapper class on carla's waypoint object.
    /// The class is used to represent descrete samples of the world map.

  private:

    carla::SharedPtr<carla::client::Waypoint> waypoint;
    std::vector<std::shared_ptr<SimpleWaypoint>> next_waypoints;
    std::shared_ptr<SimpleWaypoint> next_left_waypoint;
    std::shared_ptr<SimpleWaypoint> next_right_waypoint;

  public:

    SimpleWaypoint(carla::SharedPtr<carla::client::Waypoint> waypoint);
    ~SimpleWaypoint();

    /// Returns the location object for this waypoint.
    carla::geom::Location getLocation() const;

    /// Returns a carla::shared_ptr to carla::waypoint
    carla::SharedPtr<carla::client::Waypoint> getWaypoint() const; 

    /// Returns the list of next waypoints.
    std::vector<std::shared_ptr<SimpleWaypoint>> getNextWaypoint() const;

    /// Returns the vector along the waypoint's direction.
    carla::geom::Vector3D getVector()const;

    /// Returns the location of the waypoint as a list of x,y,z float values.
    std::vector<float> getXYZ()const;

    /// This method is used to set the next waypoints.
    int setNextWaypoint(std::vector<std::shared_ptr<SimpleWaypoint>> next_waypoints);

    /// This method is used to set the closest left waypoint for lane change
    void setLeftWaypoint(std::shared_ptr<SimpleWaypoint> waypoint);

    /// This method is used to set the closest right waypoint for lane change
    void setRightWaypoint(std::shared_ptr<SimpleWaypoint> waypoint);

    /// This method is used to get the closest left waypoint for lane change
    std::shared_ptr<SimpleWaypoint> getLeftWaypoint();
    
    /// This method is used to get the closest right waypoint for lane change
    std::shared_ptr<SimpleWaypoint> getRightWaypoint();


    /// Calculates the distance from the object's waypoint to the passed
    /// location.
    float distance(const carla::geom::Location& location)const;

    /// Returns true if the object's waypoint belongs to an intersection.
    bool checkJunction() const;

  };

}
