#include "SimpleWaypoint.h"

namespace traffic_manager {

  SimpleWaypoint::SimpleWaypoint(carla::SharedPtr<carla::client::Waypoint> _waypoint) {
    waypoint = _waypoint;
    next_left_waypoint = nullptr;
    next_right_waypoint = nullptr;
  }
  SimpleWaypoint::~SimpleWaypoint() {}

  std::vector<std::shared_ptr<SimpleWaypoint>> SimpleWaypoint::GetNextWaypoint() const {
    return next_waypoints;
  }

  carla::SharedPtr<carla::client::Waypoint> SimpleWaypoint::GetWaypoint() const {
    return waypoint;
  }

  std::shared_ptr<SimpleWaypoint> SimpleWaypoint::GetLeftWaypoint() {
    return next_left_waypoint;
  }

  std::shared_ptr<SimpleWaypoint> SimpleWaypoint::GetRightWaypoint() {
    return next_right_waypoint;
  }

  carla::geom::Location SimpleWaypoint::GetLocation() const {
    return waypoint->GetTransform().location;
  }

  carla::geom::Vector3D SimpleWaypoint::GetForwardVector() const {
    return waypoint->GetTransform().rotation.GetForwardVector();
  }

  std::vector<float> SimpleWaypoint::GetXYZ() const {
    float x = waypoint->GetTransform().location.x;
    float y = waypoint->GetTransform().location.y;
    float z = waypoint->GetTransform().location.z;
    std::vector<float> coordinates = {x, y, z};
    return coordinates;
  }

  int SimpleWaypoint::SetNextWaypoint(std::vector<std::shared_ptr<SimpleWaypoint>> waypoints) {
    next_waypoints.insert(
        std::end(next_waypoints),
        std::begin(waypoints),
        std::end(waypoints));
    return waypoints.size();
  }

  void SimpleWaypoint::SetLeftWaypoint(std::shared_ptr<SimpleWaypoint> waypoint) {
    next_left_waypoint = waypoint;
  }

  void SimpleWaypoint::SetRightWaypoint(std::shared_ptr<SimpleWaypoint> waypoint) {
    next_right_waypoint = waypoint;
  }

  float SimpleWaypoint::Distance(const carla::geom::Location &location) const {
    return waypoint->GetTransform().location.Distance(location);
  }

  bool SimpleWaypoint::CheckJunction() const {
    return waypoint->IsJunction();
  }

}
