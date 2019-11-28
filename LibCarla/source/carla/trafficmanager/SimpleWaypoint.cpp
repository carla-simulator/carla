#include "SimpleWaypoint.h"

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

  WaypointPtr SimpleWaypoint::GetWaypoint() const {
    return waypoint;
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

  uint SimpleWaypoint::SetNextWaypoint(const std::vector<SimpleWaypointPtr> &waypoints) {
    for (auto &simple_waypoint: waypoints) {
      next_waypoints.push_back(simple_waypoint);
    }
    return static_cast<uint>(waypoints.size());
  }

  void SimpleWaypoint::SetLeftWaypoint(SimpleWaypointPtr _waypoint) {

    cg::Vector3D heading_vector = waypoint->GetTransform().GetForwardVector();
    cg::Vector3D relative_vector = GetLocation() - _waypoint->GetLocation();
    if ((heading_vector.x * relative_vector.y - heading_vector.y * relative_vector.x) > 0) {
      next_left_waypoint = _waypoint;
    } else {
      throw std::invalid_argument("Argument not on the left side!");
    }
  }

  void SimpleWaypoint::SetRightWaypoint(SimpleWaypointPtr _waypoint) {

    cg::Vector3D heading_vector = waypoint->GetTransform().GetForwardVector();
    cg::Vector3D relative_vector = GetLocation() - _waypoint->GetLocation();
    if ((heading_vector.x * relative_vector.y - heading_vector.y * relative_vector.x) < 0) {
      next_right_waypoint = _waypoint;
    } else {
      throw std::invalid_argument("Argument not on the right side!");
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
    return waypoint->IsJunction();
  }

}
