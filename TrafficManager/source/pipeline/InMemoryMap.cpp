#include "InMemoryMap.h"

namespace traffic_manager {

namespace MapConstants {
  // Very important that this is less than 10^-4
  static const float ZERO_LENGTH = 0.0001f;
  static const float INFINITE_DISTANCE = std::numeric_limits<float>::max();
  static const uint LANE_CHANGE_LOOK_AHEAD = 5u;
  static const float LANE_CHANGE_ANGULAR_THRESHOLD = 0.5f;     // cos(angle)
}
  using namespace MapConstants;

  InMemoryMap::InMemoryMap(TopologyList topology) {
    _topology = topology;
  }
  InMemoryMap::~InMemoryMap() {}

  void InMemoryMap::SetUp(int sampling_resolution) {

    NodeList entry_node_list;
    NodeList exit_node_list;

    // Creating dense topology
    for (auto &pair : _topology) {

      // Looping through every topology segment
      auto begin_waypoint = pair.first;
      auto end_waypoint = pair.second;

      if (begin_waypoint->GetTransform().location.Distance(
          end_waypoint->GetTransform().location) > ZERO_LENGTH) {

        // Adding entry waypoint
        auto current_waypoint = begin_waypoint;
        dense_topology.push_back(std::make_shared<SimpleWaypoint>(current_waypoint));

        StructuredWaypoints(dense_topology.back());

        entry_node_list.push_back(dense_topology.back());

        // Populating waypoints from begin_waypoint to end_waypoint
        while (current_waypoint->GetTransform().location.Distance(
            end_waypoint->GetTransform().location) > sampling_resolution) {

          current_waypoint = current_waypoint->GetNext(sampling_resolution)[0];
          auto previous_wp = dense_topology.back();
          dense_topology.push_back(std::make_shared<SimpleWaypoint>(current_waypoint));

          StructuredWaypoints(dense_topology.back());

          previous_wp->SetNextWaypoint({dense_topology.back()});
        }

        // Adding exit waypoint
        auto previous_wp = dense_topology.back();
        dense_topology.push_back(std::make_shared<SimpleWaypoint>(end_waypoint));

        StructuredWaypoints(dense_topology.back());

        previous_wp->SetNextWaypoint({dense_topology.back()});
        exit_node_list.push_back(dense_topology.back());
      }
    }

    // Linking segments
    uint i = 0, j = 0;
    for (auto end_point : exit_node_list) {
      for (auto begin_point : entry_node_list) {
        if (end_point->Distance(begin_point->GetLocation()) < ZERO_LENGTH and i != j) {
          end_point->SetNextWaypoint({begin_point});
        }
        ++j;
      }
      ++i;
    }

    // Tying up loose ends
    // Loop through all exit nodes of topology segments,
    // connect any dangling end points to nearest entry point
    // of another topology segment
    i = 0;
    for (auto end_point : exit_node_list) {
      if (end_point->GetNextWaypoint().size() == 0) {
        j = 0;
        float min_distance = INFINITE_DISTANCE;
        SimpleWaypointPtr closest_connection;
        for (auto begin_point : entry_node_list) {
          auto new_distance = end_point->Distance(begin_point->GetLocation());
          if (new_distance < min_distance and i != j) {
            min_distance = new_distance;
            closest_connection = begin_point;
          }
          ++j;
        }
        auto end_point_vector = end_point->GetForwardVector();
        auto relative_vector = closest_connection->GetLocation() - end_point->GetLocation();
        relative_vector = relative_vector.MakeUnitVector();
        auto relative_dot = carla::geom::Math::Dot(end_point_vector, relative_vector);
        if (relative_dot < LANE_CHANGE_ANGULAR_THRESHOLD) {
          int count = LANE_CHANGE_LOOK_AHEAD;
          while (count > 0) {
            closest_connection = closest_connection->GetNextWaypoint()[0];
            --count;
          }
        }
        end_point->SetNextWaypoint({closest_connection});
      }
      ++i;
    }

    // Linking lane change connections
    for (auto simple_waypoint:dense_topology) {
      FindAndLinkLaneChange(simple_waypoint);
    }

  }

  SimpleWaypointPtr InMemoryMap::GetWaypoint(const carla::geom::Location &location) const {
    // Need more efficient code for the functionality
    SimpleWaypointPtr closest_waypoint;
    float min_distance = std::numeric_limits<float>::max();
    for (auto simple_waypoint : dense_topology) {
      float current_distance = simple_waypoint->Distance(location);
      if (current_distance < min_distance) {
        min_distance = current_distance;
        closest_waypoint = simple_waypoint;
      }
    }
    return closest_waypoint;
  }

  std::vector<SimpleWaypointPtr> InMemoryMap::GetDenseTopology() const {
    return dense_topology;
  }

  void InMemoryMap::StructuredWaypoints(SimpleWaypointPtr waypoint) {
    auto current_waypoint = waypoint->GetWaypoint();
    auto section_id = current_waypoint->GetSectionId();
    auto road_id = current_waypoint->GetRoadId();
    auto lane_id = current_waypoint->GetLaneId();

    if (road_to_waypoint.find(road_id) != road_to_waypoint.end()) {
      if (road_to_waypoint[road_id].find(section_id) != road_to_waypoint[road_id].end()) {
        if (road_to_waypoint[road_id][section_id].find(lane_id) !=
            road_to_waypoint[road_id][section_id].end()) {

          road_to_waypoint[road_id][section_id][lane_id].push_back(waypoint);
        } else {
          std::vector<SimpleWaypointPtr> lane_wp;
          lane_wp.push_back(waypoint);
          road_to_waypoint[road_id][section_id].insert({lane_id, lane_wp});
        }
      } else {
        std::vector<SimpleWaypointPtr> lane_wp;
        lane_wp.push_back(waypoint);
        LaneWaypointMap lane_map;
        lane_map.insert({lane_id, lane_wp});
        road_to_waypoint[road_id].insert({section_id, lane_map});
      }
    } else {

      std::vector<SimpleWaypointPtr> lane_wp;
      lane_wp.push_back(waypoint);
      LaneWaypointMap lane_map;
      lane_map.insert({lane_id, lane_wp});
      SectionWaypointMap section_map;
      section_map.insert({section_id, lane_map});
      road_to_waypoint.insert({road_id, section_map});
    }
  }

  void InMemoryMap::LinkLaneChangePoint(
      SimpleWaypointPtr reference_waypoint,
      WaypointPtr neighbor_waypoint,
      int side) {

    if (neighbor_waypoint != nullptr) {
      auto neighbour_road_id = neighbor_waypoint->GetRoadId();
      auto neighbour_section_id = neighbor_waypoint->GetSectionId();
      auto neighbour_lane_id = neighbor_waypoint->GetLaneId();

      // Find waypoint samples in dense topology corresponding to the
      // geo ids of the neighbor waypoint found using carla's server call
      if (road_to_waypoint.find(neighbour_road_id) != road_to_waypoint.end() &&
          (road_to_waypoint[neighbour_road_id].find(neighbour_section_id)
          != road_to_waypoint[neighbour_road_id].end()) &&
          (road_to_waypoint[neighbour_road_id][neighbour_section_id].find(neighbour_lane_id)
          != road_to_waypoint[neighbour_road_id][neighbour_section_id].end())) {

        std::vector<SimpleWaypointPtr> waypoints_to_left =
            road_to_waypoint[neighbour_road_id][neighbour_section_id][neighbour_lane_id];

        // Find the nearest sample to the neighbour waypoint to be used as a
        // local cache representative to be linked for indicating lane change
        // connection
        if (waypoints_to_left.size() > 0) {
          auto nearest_waypoint = waypoints_to_left[0];
          auto smallest_left_distance = INFINITE_DISTANCE;
          for (auto left_wp : waypoints_to_left) {
            if (reference_waypoint->Distance(left_wp->GetLocation()) < smallest_left_distance) {
              smallest_left_distance = reference_waypoint->Distance(left_wp->GetLocation());
              nearest_waypoint = left_wp;
            }
          }

          // Place appropriate lane change link
          if (side < 0) {
            reference_waypoint->SetLeftWaypoint(nearest_waypoint);
          } else if (side > 0) {
            reference_waypoint->SetRightWaypoint(nearest_waypoint);
          }
        }
      }
    }
  }

  void InMemoryMap::FindAndLinkLaneChange(SimpleWaypointPtr reference_waypoint) {
    auto raw_waypoint = reference_waypoint->GetWaypoint();
    uint8_t lane_change = static_cast<uint8_t>(raw_waypoint->GetLaneChange());
    uint8_t change_right = static_cast<uint8_t>(carla::road::element::LaneMarking::LaneChange::Right);
    uint8_t change_left = static_cast<uint8_t>(carla::road::element::LaneMarking::LaneChange::Left);

    if ((lane_change & change_right) > 0) {
      auto right_waypoint =  raw_waypoint->GetRight();
      LinkLaneChangePoint(reference_waypoint, right_waypoint, +1);
    }
    if ((lane_change & change_left) > 0) {
      auto left_waypoint =  raw_waypoint->GetLeft();
      LinkLaneChangePoint(reference_waypoint, left_waypoint, -1);
    }

  }
}
