// Implementation for SimpleWaypoint

#include "SimpleWaypoint.hpp"

namespace traffic_manager{

    SimpleWaypoint::SimpleWaypoint(carla::SharedPtr<carla::client::Waypoint> waypoint) {
        this->waypoint = waypoint;
    }
    SimpleWaypoint::~SimpleWaypoint(){}

    std::vector<std::shared_ptr<SimpleWaypoint>> SimpleWaypoint::getNextWaypoint() {
        return this->next_waypoints;
    }

    carla::geom::Location SimpleWaypoint::getLocation() {
        return this->waypoint->GetTransform().location;
    }

    carla::geom::Vector3D SimpleWaypoint::getVector(){
        return waypoint->GetTransform().rotation.GetForwardVector();
    }

    std::vector<float> SimpleWaypoint::getXYZ(){
        float x = waypoint->GetTransform().location.x;
        float y = waypoint->GetTransform().location.y;
        float z = waypoint->GetTransform().location.z;
        std::vector<float> coordinates = {x,y,z};
        return coordinates;
    }

    int SimpleWaypoint::setNextWaypoint(std::vector<std::shared_ptr<SimpleWaypoint>> waypoints) {
        this->next_waypoints.insert(
            std::end(this->next_waypoints),
            std::begin(waypoints),
            std::end(waypoints));
        return waypoints.size();
    }

    float SimpleWaypoint::distance(carla::geom::Location location) {
        return this->waypoint->GetTransform().location.Distance(location);
    }

    bool SimpleWaypoint::checkJunction(){
        return this->waypoint->IsJunction();
    }
}
