// Implementation for SimpleWaypoint

#include "SimpleWaypoint.h"

namespace traffic_manager {

  SimpleWaypoint::SimpleWaypoint(carla::SharedPtr<carla::client::Waypoint> waypoint) {
    this->waypoint = waypoint;
  }
  SimpleWaypoint::~SimpleWaypoint() {}

  std::vector<std::shared_ptr<SimpleWaypoint>> SimpleWaypoint::getNextWaypoint() const {
    return this->next_waypoints;
  }

  carla::SharedPtr<carla::client::Waypoint> SimpleWaypoint::getWaypoint() const{
    return this->waypoint;
  }

  std::shared_ptr<SimpleWaypoint> SimpleWaypoint::getLeftWaypoint(){
    return this->next_left_waypoint;
  }

  std::shared_ptr<SimpleWaypoint> SimpleWaypoint::getRightWaypoint(){
    return this->next_right_waypoint;
  }

  carla::geom::Location SimpleWaypoint::getLocation() const {
    return this->waypoint->GetTransform().location;
  }

  carla::geom::Vector3D SimpleWaypoint::getVector() const {
    return waypoint->GetTransform().rotation.GetForwardVector();
  }

  std::vector<float> SimpleWaypoint::getXYZ() const {
    float x = waypoint->GetTransform().location.x;
    float y = waypoint->GetTransform().location.y;
    float z = waypoint->GetTransform().location.z;
    std::vector<float> coordinates = {x, y, z};
    return coordinates;
  }

  int SimpleWaypoint::setNextWaypoint(std::vector<std::shared_ptr<SimpleWaypoint>> waypoints) {
    this->next_waypoints.insert(
        std::end(this->next_waypoints),
        std::begin(waypoints),
        std::end(waypoints));
    return waypoints.size();
  }

  void SimpleWaypoint::setLeftWaypoint(std::shared_ptr<SimpleWaypoint> waypoint){
    this->next_left_waypoint = waypoint;
  }

  void SimpleWaypoint::setRightWaypoint(std::shared_ptr<SimpleWaypoint> waypoint){
    this->next_right_waypoint = waypoint;
  }

  float SimpleWaypoint::distance(const carla::geom::Location& location) const {
    return this->waypoint->GetTransform().location.Distance(location);
  }

  bool SimpleWaypoint::checkJunction() const {
    return this->waypoint->IsJunction();
  }

  
}
