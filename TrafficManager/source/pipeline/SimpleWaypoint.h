#pragma once

#include <memory.h>

#include "carla/client/Waypoint.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/geom/Vector3D.h"
#include "carla/Memory.h"

namespace traffic_manager {

namespace cc = carla::client;
namespace cg = carla::geom;
  using WaypointPtr = carla::SharedPtr<cc::Waypoint>;

  /// This is a simple wrapper class on carla's waypoint object
  /// The class is used to represent descrete samples of the world map
  class SimpleWaypoint {

    using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;

  private:

    /// Pointer to carla's waypoint object around which this class wraps around
    WaypointPtr waypoint;
    /// List of pointers to next connecting waypoints
    std::vector<SimpleWaypointPtr> next_waypoints;
    /// Pointer to left lane change waypoint
    SimpleWaypointPtr next_left_waypoint;
    /// Pointer to right lane change waypoint
    SimpleWaypointPtr next_right_waypoint;

  public:

    SimpleWaypoint(WaypointPtr _waypoint);
    ~SimpleWaypoint();

    /// Returns the location object for this waypoint
    cg::Location GetLocation() const;

    /// Returns a carla::shared_ptr to carla::waypoint
    WaypointPtr GetWaypoint() const;

    /// Returns the list of next waypoints
    std::vector<SimpleWaypointPtr> GetNextWaypoint() const;

    /// Returns the vector along the waypoint's direction
    cg::Vector3D GetForwardVector() const;

    /// Returns the location of the waypoint as a list of x,y,z float values
    std::vector<float> GetXYZ() const;

    /// This method is used to set the next waypoints
    int SetNextWaypoint(const std::vector<SimpleWaypointPtr> &next_waypoints);

    /// This method is used to set the closest left waypoint for lane change
    void SetLeftWaypoint(SimpleWaypointPtr waypoint);

    /// This method is used to set the closest right waypoint for lane change
    void SetRightWaypoint(SimpleWaypointPtr waypoint);

    /// This method is used to get the closest left waypoint for lane change
    SimpleWaypointPtr GetLeftWaypoint();

    /// This method is used to get the closest right waypoint for lane change
    SimpleWaypointPtr GetRightWaypoint();

    /// Calculates the distance from the object's waypoint to the passed
    /// location
    float Distance(const cg::Location &location) const;

    /// Calculates distance the other SimpleWaypoint object
    float Distance(const SimpleWaypointPtr &other) const;

    /// Calculates the square of the distance to given location
    float DistanceSquared(const cg::Location &location) const;

    /// Calculates the square of the distance to other waypoint
    float DistanceSquared(const SimpleWaypointPtr &other) const;

    /// Returns true if the object's waypoint belongs to an intersection
    bool CheckJunction() const;

  };

}
