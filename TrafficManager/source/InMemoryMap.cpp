// Implementation for an in memory descrete map representation

#include "InMemoryMap.h"

namespace traffic_manager {

  const float ZERO_LENGTH = 0.0001;   // Very important that this is less than
                                      // 10^-4
  const float INFINITE_DISTANCE = 1000000;
  const int LANE_CHANGE_LOOK_AHEAD = 5;
  const float LANE_CHANGE_ANGULAR_THRESHOLD = 0.5;   // cos of the angle

  InMemoryMap::InMemoryMap(traffic_manager::TopologyList topology) {
    this->topology = topology;
  }
  InMemoryMap::~InMemoryMap() {}

  void InMemoryMap::setUp(int sampling_resolution) {
    /// Creating dense topology
    for (auto &pair : this->topology) {

      /// Looping through every topology segment
      auto begin_waypoint = pair.first;
      auto end_waypoint = pair.second;

      if (begin_waypoint->GetTransform().location.Distance(
          end_waypoint->GetTransform().location) > ZERO_LENGTH) {

        /// Adding entry waypoint
        auto current_waypoint = begin_waypoint;
        this->dense_topology.push_back(std::make_shared<SimpleWaypoint>(current_waypoint));
        
        structuredWaypoints(dense_topology.back());

        this->entry_node_list.push_back(this->dense_topology.back());

        /// Populating waypoints from begin_waypoint to end_waypoint
        while (current_waypoint->GetTransform().location.Distance(
            end_waypoint->GetTransform().location) > sampling_resolution) {

          current_waypoint = current_waypoint->GetNext(sampling_resolution)[0];
          auto previous_wp = this->dense_topology.back();
          this->dense_topology.push_back(std::make_shared<SimpleWaypoint>(current_waypoint));

          structuredWaypoints(dense_topology.back());

          previous_wp->setNextWaypoint({this->dense_topology.back()});
        }

        /// Adding exit waypoint
        auto previous_wp = this->dense_topology.back();
        this->dense_topology.push_back(std::make_shared<SimpleWaypoint>(end_waypoint));

        structuredWaypoints(dense_topology.back());

        previous_wp->setNextWaypoint({this->dense_topology.back()});
        this->exit_node_list.push_back(this->dense_topology.back());
      }
    }

    /// Linking segments
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

    /// Tying up loose ends
    i = 0;
    for (auto end_point : this->exit_node_list) {
      if (end_point->getNextWaypoint().size() == 0) {
        j = 0;
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
        auto relative_dot = end_point_vector.x * relative_vector.x +
            end_point_vector.y * relative_vector.y +
            end_point_vector.z * relative_vector.z;
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

    // Linking lane change connections
    for(auto simple_waypoint:dense_topology) {
      FindAndLinkLaneChange(simple_waypoint);
    }  

  }

  std::shared_ptr<SimpleWaypoint> InMemoryMap::getWaypoint(const carla::geom::Location &location) const {
    /// Dumb first draft implementation. Need more efficient code for the
    /// functionality
    std::shared_ptr<SimpleWaypoint> closest_waypoint;
    float min_distance = std::numeric_limits<float>::max();
    for (auto simple_waypoint : this->dense_topology) {
      float current_distance = simple_waypoint->distance(location);
      if (current_distance < min_distance) {
        min_distance = current_distance;
        closest_waypoint = simple_waypoint;
      }
    }
    return closest_waypoint;
  }

  std::vector<std::shared_ptr<SimpleWaypoint>> InMemoryMap::get_dense_topology() const {
    return this->dense_topology;
  }


  void InMemoryMap::structuredWaypoints(std::shared_ptr<SimpleWaypoint> waypoint)
  { 
    auto current_waypoint = waypoint->getWaypoint();
    auto section_id = current_waypoint->GetSectionId();
    auto road_id = current_waypoint->GetRoadId();
    auto lane_id = current_waypoint->GetLaneId();

    if (waypoint_structure.find(road_id) != waypoint_structure.end()) {
      if(waypoint_structure[road_id].find(section_id) != waypoint_structure[road_id].end()) {
        if(waypoint_structure[road_id][section_id].find(lane_id) != waypoint_structure[road_id][section_id].end()) {
            
            waypoint_structure[road_id][section_id][lane_id].push_back(waypoint);
            // std::cout << "Road id : " << road_id << "\tSection Id : " << section_id << "\tLane id : " << lane_id << "\t#Waypoints : " << 
            //   waypoint_structure[road_id][section_id][lane_id].size() << std::endl;
        } else {
          std::vector<std::shared_ptr<SimpleWaypoint>> lane_wp;
          lane_wp.push_back(waypoint);
          waypoint_structure[road_id][section_id].insert(
            std::pair<int32_t,std::vector<std::shared_ptr<SimpleWaypoint>>>(lane_id,lane_wp)
          );
            // std::cout << "Road id : " << road_id << "\tSection Id : " << section_id << "\tLane id : " << lane_id << "\t#Waypoints : " << 
            //   waypoint_structure[road_id][section_id][lane_id].size() << std::endl;
        }
      } else {
        std::vector<std::shared_ptr<SimpleWaypoint>> lane_wp;
        lane_wp.push_back(waypoint);
        std::map<int32_t,std::vector<std::shared_ptr<SimpleWaypoint>>> lane_map;
        lane_map.insert(
          std::pair<int32_t,std::vector<std::shared_ptr<SimpleWaypoint>>>(lane_id,lane_wp)
        ); 
        waypoint_structure[road_id].insert(
          std::pair<uint32_t, std::map<int32_t, std::vector<std::shared_ptr<SimpleWaypoint>>>>(section_id, lane_map)
        );
        // std::cout << "Road id : " << road_id << "\tSection Id : " << section_id << "\tLane id : " << lane_id << "\t#Waypoints : " << 
        //       waypoint_structure[road_id][section_id][lane_id].size() << std::endl;              
      }
    } else {

      std::vector<std::shared_ptr<SimpleWaypoint>> lane_wp;
      lane_wp.push_back(waypoint);
      std::map<int32_t,std::vector<std::shared_ptr<SimpleWaypoint>>> lane_map;
      lane_map.insert(
        std::pair<int32_t,std::vector<std::shared_ptr<SimpleWaypoint>>>(lane_id,lane_wp)
      );
      std::map<uint32_t, std::map<int32_t, std::vector<std::shared_ptr<SimpleWaypoint>>>> section_map;
      section_map.insert(
        std::pair<uint32_t, std::map<int32_t, std::vector<std::shared_ptr<SimpleWaypoint>>>>(section_id, lane_map)
      );
      waypoint_structure.insert(
        std::pair<uint32_t, std::map<uint32_t, std::map<int32_t, std::vector<std::shared_ptr<SimpleWaypoint>>>>>(road_id, section_map)
      );
      // std::cout << "Road id : " << road_id << "\tSection Id : " << section_id << "\tLane id : " << lane_id << "\t#Waypoints : " << 
      //   waypoint_structure[road_id][section_id][lane_id].size() << std::endl;
    }
  }

  void InMemoryMap::LinkLaneChangePoint(
    std::shared_ptr<SimpleWaypoint> reference_waypoint,
    carla::SharedPtr<carla::client::Waypoint> neighbor_waypoint
  ){

    if (neighbor_waypoint != nullptr) {
      auto neighbour_road_id = neighbor_waypoint->GetRoadId();
      auto neigbhour_section_id = neighbor_waypoint->GetSectionId();
      auto neigbhour_lane_id = neighbor_waypoint->GetLaneId();

      if (
        waypoint_structure.find(neighbour_road_id) != waypoint_structure.end()
        and
        waypoint_structure[neighbour_road_id].find(neigbhour_section_id)
          != waypoint_structure[neighbour_road_id].end()
        and
        waypoint_structure[neighbour_road_id][neigbhour_section_id].find(neigbhour_lane_id)
          != waypoint_structure[neighbour_road_id][neigbhour_section_id].end()
      ) {

        std::vector<std::shared_ptr<SimpleWaypoint>> waypoints_to_left
          = waypoint_structure
            [neighbor_waypoint->GetRoadId()]
            [neighbor_waypoint->GetSectionId()]
            [neighbor_waypoint->GetLaneId()];

        if (waypoints_to_left.size() > 0) {
          auto nearest_left_waypoint = waypoints_to_left[0];
          auto smallest_left_distance = INFINITE_DISTANCE;
          for(auto left_wp : waypoints_to_left) {
            if(reference_waypoint->distance(left_wp->getLocation()) < smallest_left_distance)
            {
              smallest_left_distance = reference_waypoint->distance(left_wp->getLocation());
              nearest_left_waypoint = left_wp;
            }
          }
          reference_waypoint->setLeftWaypoint(nearest_left_waypoint);
        }
      }
    }
  }

  void InMemoryMap::FindAndLinkLaneChange(std::shared_ptr<SimpleWaypoint> reference_waypoint) {
    auto raw_waypoint = reference_waypoint->getWaypoint();
    uint8_t lane_change = static_cast<uint8_t>(raw_waypoint->GetLaneChange());
    uint8_t change_right = static_cast<uint8_t>(carla::road::element::LaneMarking::LaneChange::Right);
    uint8_t change_left = static_cast<uint8_t>(carla::road::element::LaneMarking::LaneChange::Left);

    if ((lane_change & change_right) > 0) {
      auto right_waypoint =  raw_waypoint->GetRight();
      LinkLaneChangePoint(reference_waypoint, right_waypoint);
    }
    if ((lane_change & change_left) > 0) {
      auto left_waypoint =  raw_waypoint->GetLeft();
      LinkLaneChangePoint(reference_waypoint, left_waypoint);
     }
    
  }
}
