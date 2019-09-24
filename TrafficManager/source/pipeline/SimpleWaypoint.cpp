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

  std::vector<float> SimpleWaypoint::GetXYZ() const {
    float x = waypoint->GetTransform().location.x;
    float y = waypoint->GetTransform().location.y;
    float z = waypoint->GetTransform().location.z;
    std::vector<float> coordinates = {x, y, z};
    return coordinates;
  }

  int SimpleWaypoint::SetNextWaypoint(const std::vector<SimpleWaypointPtr> &waypoints) {
    for (auto &simple_waypoint: waypoints) {
      next_waypoints.push_back(simple_waypoint);
    }
    return waypoints.size();
  }

  void SimpleWaypoint::SetLeftWaypoint(SimpleWaypointPtr waypoint) {
    next_left_waypoint = waypoint;
  }

  void SimpleWaypoint::SetRightWaypoint(SimpleWaypointPtr waypoint) {
    next_right_waypoint = waypoint;
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
