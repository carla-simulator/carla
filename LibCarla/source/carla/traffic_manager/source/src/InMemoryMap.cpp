// Implementation for an in memory descrete map representation

#include "InMemoryMap.hpp"

namespace traffic_manager {

    const float ZERO_LENGTH = 0.0001; // Very important that this is less than 10^-4
    const float INFINITE_DISTANCE = 1000000;
    const int LANE_CHANGE_LOOK_AHEAD = 5;
    const float LANE_CHANGE_ANGULAR_THRESHOLD = 0.5; // cos of the angle

    InMemoryMap::InMemoryMap(traffic_manager::TopologyList topology) {
        this->topology = topology;
    }
    InMemoryMap::~InMemoryMap(){}

    void InMemoryMap::setUp(int sampling_resolution){
        // Creating dense topology
        for(auto &pair : this->topology) {

            // Looping through every topology segment
            auto begin_waypoint = pair.first;
            auto end_waypoint = pair.second;

            if (begin_waypoint->GetTransform().location.Distance(
                    end_waypoint->GetTransform().location) > ZERO_LENGTH) {

                // Adding entry waypoint
                auto current_waypoint = begin_waypoint;
                this->dense_topology.push_back(std::make_shared<SimpleWaypoint>(current_waypoint));
                this->entry_node_list.push_back(this->dense_topology.back());

                // Populating waypoints from begin_waypoint to end_waypoint
                while (current_waypoint->GetTransform().location.Distance(
                        end_waypoint->GetTransform().location) > sampling_resolution) {

                    current_waypoint = current_waypoint->GetNext(sampling_resolution)[0];
                    auto previous_wp = this->dense_topology.back();
                    this->dense_topology.push_back(std::make_shared<SimpleWaypoint>(current_waypoint));
                    previous_wp->setNextWaypoint({this->dense_topology.back()});
                }

                // Adding exit waypoint
                auto previous_wp = this->dense_topology.back();
                this->dense_topology.push_back(std::make_shared<SimpleWaypoint>(end_waypoint));
                previous_wp->setNextWaypoint({this->dense_topology.back()});
                this->exit_node_list.push_back(this->dense_topology.back());
            }
        }

        // Linking segments
        int i = 0, j = 0;
        for (auto end_point : this->exit_node_list) {
            for (auto begin_point : this->entry_node_list) {
                if (end_point->distance(begin_point->getLocation()) < ZERO_LENGTH and i != j) {
                    end_point->setNextWaypoint({begin_point});
                }
                j++;
            }
            i++;
        }

        // Tying up loose ends
        i = 0;
        for (auto end_point : this->exit_node_list) {
            if (end_point->getNextWaypoint().size() == 0) {
                j=0;
                float min_distance = INFINITE_DISTANCE;
                std::shared_ptr<traffic_manager::SimpleWaypoint> closest_connection;
                for (auto begin_point : this->entry_node_list) {
                    auto new_distance = end_point->distance(begin_point->getLocation());
                    if (new_distance < min_distance and i != j) {
                        min_distance = new_distance;
                        closest_connection = begin_point;
                    }
                    j++;
                }
                auto end_point_vector = end_point->getVector();
                auto relative_vector = closest_connection->getLocation() - end_point->getLocation();
                relative_vector = relative_vector.MakeUnitVector();
                auto relative_dot = end_point_vector.x*relative_vector.x
                    + end_point_vector.y*relative_vector.y
                    + end_point_vector.z*relative_vector.z;
                if (relative_dot < LANE_CHANGE_ANGULAR_THRESHOLD) {   
                    int count = LANE_CHANGE_LOOK_AHEAD;
                    while (count > 0) {
                        closest_connection = closest_connection->getNextWaypoint()[0];
                        count--;
                    }
                }
                end_point->setNextWaypoint({closest_connection});
            }
            i++;
        }
    }

    std::shared_ptr<SimpleWaypoint> InMemoryMap::getWaypoint(carla::geom::Location location) {
        /* Dumb first draft implementation. Need more efficient code for the functionality */
        std::shared_ptr<SimpleWaypoint> closest_waypoint;
        float min_distance = std::numeric_limits<float>::max();
        for(auto simple_waypoint : this->dense_topology){
            float current_distance = simple_waypoint->distance(location);
            if (current_distance < min_distance) {
                min_distance = current_distance;
                closest_waypoint = simple_waypoint;
            }
        }
        return closest_waypoint;
    }

    std::vector<std::shared_ptr<SimpleWaypoint>> InMemoryMap::get_dense_topology() {
        return this->dense_topology;
    }
}
