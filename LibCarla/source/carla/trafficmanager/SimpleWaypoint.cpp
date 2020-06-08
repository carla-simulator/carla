// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/geom/Math.h"

#include "carla/trafficmanager/SimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;

  SimpleWaypoint::SimpleWaypoint(WaypointPtr _waypoint) {
    waypoint = _waypoint;
    next_left_waypoint = nullptr;
    next_right_waypoint = nullptr;
  }
  SimpleWaypoint::~SimpleWaypoint() {}

  std::vector<SimpleWaypointPtr> SimpleWaypoint::GetNextWaypoint() const {
    return next_waypoints;
  }

  std::vector<SimpleWaypointPtr> SimpleWaypoint::GetPreviousWaypoint() const {
    return previous_waypoints;
  }

  WaypointPtr SimpleWaypoint::GetWaypoint() const {
    return waypoint;
  }

  uint64_t SimpleWaypoint::GetId() const {
    return waypoint->GetId();
  }

  SimpleWaypointPtr SimpleWaypoint::GetLeftWaypoint() {
    return next_left_waypoint;
  }

  SimpleWaypointPtr SimpleWaypoint::GetRightWaypoint() {
    return next_right_waypoint;
  }

  cg::Location SimpleWaypoint::GetLocation() const {
    return waypoint->GetTransform().location;
  }

  cg::Vector3D SimpleWaypoint::GetForwardVector() const {
    return waypoint->GetTransform().rotation.GetForwardVector();
  }

  uint64_t SimpleWaypoint::SetNextWaypoint(const std::vector<SimpleWaypointPtr> &waypoints) {
    for (auto &simple_waypoint: waypoints) {
      next_waypoints.push_back(simple_waypoint);
    }
    return static_cast<uint64_t>(waypoints.size());
  }

  uint64_t SimpleWaypoint::SetPreviousWaypoint(const std::vector<SimpleWaypointPtr> &waypoints) {
    for (auto &simple_waypoint: waypoints) {
      previous_waypoints.push_back(simple_waypoint);
    }
    return static_cast<uint64_t>(waypoints.size());
  }

  void SimpleWaypoint::SetLeftWaypoint(SimpleWaypointPtr &_waypoint) {

    const cg::Vector3D heading_vector = waypoint->GetTransform().GetForwardVector();
    const cg::Vector3D relative_vector = GetLocation() - _waypoint->GetLocation();
    if ((heading_vector.x * relative_vector.y - heading_vector.y * relative_vector.x) > 0.0f) {
      next_left_waypoint = _waypoint;
    }
  }

  void SimpleWaypoint::SetRightWaypoint(SimpleWaypointPtr &_waypoint) {

    const cg::Vector3D heading_vector = waypoint->GetTransform().GetForwardVector();
    const cg::Vector3D relative_vector = GetLocation() - _waypoint->GetLocation();
    if ((heading_vector.x * relative_vector.y - heading_vector.y * relative_vector.x) < 0.0f) {
      next_right_waypoint = _waypoint;
    }
  }

  float SimpleWaypoint::Distance(const cg::Location &location) const {
    return GetLocation().Distance(location);
  }

  float SimpleWaypoint::Distance(const SimpleWaypointPtr &other) const {
    return GetLocation().Distance(other->GetLocation());
  }

  float SimpleWaypoint::DistanceSquared(const cg::Location &location) const {
    return cg::Math::DistanceSquared(GetLocation(), location);
  }

  float SimpleWaypoint::DistanceSquared(const SimpleWaypointPtr &other) const {
    return cg::Math::DistanceSquared(GetLocation(), other->GetLocation());
  }

  bool SimpleWaypoint::CheckJunction() const {
    return _is_junction;
  }

  void SimpleWaypoint::SetIsJunction(bool value) {
    _is_junction = value;
  }

  bool SimpleWaypoint::CheckIntersection() const {
    return (next_waypoints.size() > 1);
  }

  void SimpleWaypoint::SetGeodesicGridId(GeoGridId _geodesic_grid_id) {
    geodesic_grid_id = _geodesic_grid_id;
  }

  GeoGridId SimpleWaypoint::GetGeodesicGridId() {
    GeoGridId grid_id;
    if (waypoint->IsJunction()) {
      grid_id = waypoint->GetJunctionId();
    } else {
      grid_id = geodesic_grid_id;
    }
    return grid_id;
  }

  GeoGridId SimpleWaypoint::GetJunctionId() const {
    return waypoint->GetJunctionId();
  }

  cg::Transform SimpleWaypoint::GetTransform() const {
    return waypoint->GetTransform();
  }

} // namespace traffic_manager
} // namespace carla
