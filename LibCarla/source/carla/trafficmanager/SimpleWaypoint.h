// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory.h>

#include "carla/client/Waypoint.h"
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"
#include "carla/Memory.h"
#include "carla/road/RoadTypes.h"

namespace carla {
namespace traffic_manager {

  namespace cc = carla::client;
  namespace cg = carla::geom;
  using WaypointPtr = carla::SharedPtr<cc::Waypoint>;
  using GeoGridId = carla::road::JuncId;
  enum class RoadOption : uint8_t {
    Void = 0,
    Left = 1,
    Right = 2,
    Straight = 3,
    LaneFollow = 4,
    ChangeLaneLeft = 5,
    ChangeLaneRight = 6,
    RoadEnd = 7
  };

  /// This is a simple wrapper class on Carla's waypoint object.
  /// The class is used to represent discrete samples of the world map.
  class SimpleWaypoint {

    using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;

  private:

    /// Pointer to Carla's waypoint object around which this class wraps around.
    WaypointPtr waypoint;
    /// List of pointers to next connecting waypoints.
    std::vector<SimpleWaypointPtr> next_waypoints;
    /// List of pointers to previous connecting waypoints.
    std::vector<SimpleWaypointPtr> previous_waypoints;
    /// Pointer to left lane change waypoint.
    SimpleWaypointPtr next_left_waypoint;
    /// Pointer to right lane change waypoint.
    SimpleWaypointPtr next_right_waypoint;
    /// RoadOption for the actual waypoint.
    RoadOption road_option = RoadOption::Void;
    /// Integer placing the waypoint into a geodesic grid.
    GeoGridId geodesic_grid_id = 0;
    // Boolean to hold if the waypoint belongs to a junction
    bool _is_junction = false;

  public:

    SimpleWaypoint(WaypointPtr _waypoint);
    ~SimpleWaypoint();

    /// Returns the location object for this waypoint.
    cg::Location GetLocation() const;

    /// Returns a carla::shared_ptr to carla::waypoint.
    WaypointPtr GetWaypoint() const;

    /// Returns the list of next waypoints.
    std::vector<SimpleWaypointPtr> GetNextWaypoint() const;

    /// Returns the list of previous waypoints.
    std::vector<SimpleWaypointPtr> GetPreviousWaypoint() const;

    /// Returns the vector along the waypoint's direction.
    cg::Vector3D GetForwardVector() const;

    /// Returns the unique id for the waypoint.
    uint64_t GetId() const;

    /// This method is used to set the next waypoints.
    uint64_t SetNextWaypoint(const std::vector<SimpleWaypointPtr> &next_waypoints);

    /// This method is used to set the previous waypoints.
    uint64_t SetPreviousWaypoint(const std::vector<SimpleWaypointPtr> &next_waypoints);

    /// This method is used to set the closest left waypoint for a lane change.
    void SetLeftWaypoint(SimpleWaypointPtr &waypoint);

    /// This method is used to set the closest right waypoint for a lane change.
    void SetRightWaypoint(SimpleWaypointPtr &waypoint);

    /// This method is used to get the closest left waypoint for a lane change.
    SimpleWaypointPtr GetLeftWaypoint();

    /// This method is used to get the closest right waypoint for a lane change.
    SimpleWaypointPtr GetRightWaypoint();

    /// Accessor methods for geodesic grid id.
    void SetGeodesicGridId(GeoGridId _geodesic_grid_id);
    GeoGridId GetGeodesicGridId();

    /// Method to retreive junction id of the waypoint.
    GeoGridId GetJunctionId() const;

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

    /// This method is used to set whether the waypoint belongs to a junction.
    void SetIsJunction(bool value);

    /// Returns true if the object's waypoint belongs to an intersection (Doesn't use OpenDrive).
    bool CheckIntersection() const;

    /// Return transform object for the current waypoint.
    cg::Transform GetTransform() const;

    // Accessor methods for road option.
    void SetRoadOption(RoadOption _road_option);
    RoadOption GetRoadOption();
  };

} // namespace traffic_manager
} // namespace carla
