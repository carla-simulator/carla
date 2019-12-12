// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <stdexcept>
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

  /// This is a simple wrapper class on Carla's waypoint object.
  /// The class is used to represent discrete samples of the world map.
  class SimpleWaypoint {

    using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;

  private:

    /// Pointer to Carla's waypoint object around which this class wraps around.
    WaypointPtr waypoint;
    /// List of pointers to next connecting waypoints.
    std::vector<SimpleWaypointPtr> next_waypoints;
    /// Pointer to left lane change waypoint.
    SimpleWaypointPtr next_left_waypoint;
    /// Pointer to right lane change waypoint.
    SimpleWaypointPtr next_right_waypoint;

  public:

    SimpleWaypoint(WaypointPtr _waypoint);
    ~SimpleWaypoint();

    /// Returns the location object for this waypoint.
    cg::Location GetLocation() const;

    /// Returns a carla::shared_ptr to carla::waypoint.
    WaypointPtr GetWaypoint() const;

    /// Returns the list of next waypoints.
    std::vector<SimpleWaypointPtr> GetNextWaypoint() const;

    /// Returns the vector along the waypoint's direction.
    cg::Vector3D GetForwardVector() const;

    /// Returns the unique id for the waypoint.
    uint64_t GetId() const;

    /// This method is used to set the next waypoints.
    uint64_t SetNextWaypoint(const std::vector<SimpleWaypointPtr> &next_waypoints);

    /// This method is used to set the closest left waypoint for a lane change.
    void SetLeftWaypoint(SimpleWaypointPtr waypoint);

    /// This method is used to set the closest right waypoint for a lane change.
    void SetRightWaypoint(SimpleWaypointPtr waypoint);

    /// This method is used to get the closest left waypoint for a lane change.
    SimpleWaypointPtr GetLeftWaypoint();

    /// This method is used to get the closest right waypoint for a lane change.
    SimpleWaypointPtr GetRightWaypoint();

    /// Calculates the distance from the object's waypoint to the passed
    /// location.
    float Distance(const cg::Location &location) const;

    /// Calculates the distance the other SimpleWaypoint object.
    float Distance(const SimpleWaypointPtr &other) const;

    /// Calculates the square of the distance to given location.
    float DistanceSquared(const cg::Location &location) const;

    /// Calculates the square of the distance to other waypoints.
    float DistanceSquared(const SimpleWaypointPtr &other) const;

    /// Returns true if the object's waypoint belongs to an intersection.
    bool CheckJunction() const;

    /// Returns true if the object's waypoint belongs to an intersection (Doesn't use OpenDrive).
    bool CheckIntersection() const;

  };

} // namespace traffic_manager
