#pragma once

#include <memory.h>

#include "carla/client/Waypoint.h"
#include "carla/geom/Location.h"
#include "carla/geom/Vector3D.h"
#include "carla/Memory.h"

namespace traffic_manager {

  /// This is a simple wrapper class on carla's waypoint object.
  /// The class is used to represent descrete samples of the world map.
  class SimpleWaypoint {

  private:

    /// Pointer to carla's waypoint object around which this class wraps around
    carla::SharedPtr<carla::client::Waypoint> waypoint;
    /// List of pointers to next connecting waypoints
    std::vector<std::shared_ptr<SimpleWaypoint>> next_waypoints;
    /// Pointer to left lane change waypoint
    std::shared_ptr<SimpleWaypoint> next_left_waypoint;
    /// Pointer to right lane change waypoint
    std::shared_ptr<SimpleWaypoint> next_right_waypoint;

  public:

    SimpleWaypoint(carla::SharedPtr<carla::client::Waypoint> _waypoint);
    ~SimpleWaypoint();

    /// Returns the location object for this waypoint.
    carla::geom::Location GetLocation() const;

    /// Returns a carla::shared_ptr to carla::waypoint
    carla::SharedPtr<carla::client::Waypoint> GetWaypoint() const;

    /// Returns the list of next waypoints.
    std::vector<std::shared_ptr<SimpleWaypoint>> GetNextWaypoint() const;

    /// Returns the vector along the waypoint's direction.
    carla::geom::Vector3D GetForwardVector() const;

    /// Returns the location of the waypoint as a list of x,y,z float values.
    std::vector<float> GetXYZ() const;

    /// This method is used to set the next waypoints.
    int SetNextWaypoint(std::vector<std::shared_ptr<SimpleWaypoint>> next_waypoints);

    /// This method is used to set the closest left waypoint for lane change
    void SetLeftWaypoint(std::shared_ptr<SimpleWaypoint> waypoint);

    /// This method is used to set the closest right waypoint for lane change
    void SetRightWaypoint(std::shared_ptr<SimpleWaypoint> waypoint);

    /// This method is used to get the closest left waypoint for lane change
    std::shared_ptr<SimpleWaypoint> GetLeftWaypoint();

    /// This method is used to get the closest right waypoint for lane change
    std::shared_ptr<SimpleWaypoint> GetRightWaypoint();

    /// Calculates the distance from the object's waypoint to the passed
    /// location.
    float Distance(const carla::geom::Location &location) const;

    /// Returns true if the object's waypoint belongs to an intersection.
    bool CheckJunction() const;

  };

}
