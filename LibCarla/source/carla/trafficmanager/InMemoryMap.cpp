#include "InMemoryMap.h"

namespace traffic_manager {

namespace MapConstants {
  // Very important that this is less than 10^-4.
  static const float ZERO_LENGTH = 0.0001f;
  static const float INFINITE_DISTANCE = std::numeric_limits<float>::max();
  static const uint LANE_CHANGE_LOOK_AHEAD = 5u;
  // Cosine of the angle.
  static const float LANE_CHANGE_ANGULAR_THRESHOLD = 0.5f;
}
  using namespace MapConstants;

  InMemoryMap::InMemoryMap(TopologyList topology) {
    _topology = topology;
  }
  InMemoryMap::~InMemoryMap() {}

  void InMemoryMap::SetUp(int sampling_resolution) {

    NodeList entry_node_list;
    NodeList exit_node_list;

    auto distance_squared = [](cg::Location l1, cg::Location l2) {
          return cg::Math::DistanceSquared(l1, l2);
        };
    auto square = [](float input) {return std::pow(input, 2);};

    // Creating dense topology.
    for (auto &pair : _topology) {

      // Looping through every topology segment.
      WaypointPtr begin_waypoint = pair.first;
      WaypointPtr end_waypoint = pair.second;
      cg::Location begin_location = begin_waypoint->GetTransform().location;
      cg::Location end_location = end_waypoint->GetTransform().location;

      if (distance_squared(begin_location, end_location) > square(ZERO_LENGTH)) {

        // Adding entry waypoint.
        WaypointPtr current_waypoint = begin_waypoint;
        dense_topology.push_back(std::make_shared<SimpleWaypoint>(current_waypoint));

        StructuredWaypoints(dense_topology.back());

        entry_node_list.push_back(dense_topology.back());

        // Populating waypoints from begin_waypoint to end_waypoint.
        while (distance_squared(current_waypoint->GetTransform().location,
            end_location) > square(sampling_resolution)) {

          current_waypoint = current_waypoint->GetNext(sampling_resolution)[0];
          SimpleWaypointPtr previous_wp = dense_topology.back();
          dense_topology.push_back(std::make_shared<SimpleWaypoint>(current_waypoint));

          StructuredWaypoints(dense_topology.back());

          previous_wp->SetNextWaypoint({dense_topology.back()});
        }

        // Adding exit waypoint.
        SimpleWaypointPtr previous_wp = dense_topology.back();
        dense_topology.push_back(std::make_shared<SimpleWaypoint>(end_waypoint));

        StructuredWaypoints(dense_topology.back());

        previous_wp->SetNextWaypoint({dense_topology.back()});
        exit_node_list.push_back(dense_topology.back());
      }
    }

    // Linking segments.
    uint i = 0, j = 0;
    for (SimpleWaypointPtr end_point : exit_node_list) {
      for (SimpleWaypointPtr begin_point : entry_node_list) {
        if (end_point->DistanceSquared(begin_point) < square(ZERO_LENGTH) and i != j) {
          end_point->SetNextWaypoint({begin_point});
        }
        ++j;
      }
      ++i;
    }

    // Tying up loose ends.
    // Loop through all exit nodes of topology segments,
    // connect any dangling endpoints to the nearest entry point
    // of another topology segment.
    i = 0;
    for (auto &end_point : exit_node_list) {
      if (end_point->GetNextWaypoint().size() == 0) {
        j = 0;
        float min_distance = INFINITE_DISTANCE;
        SimpleWaypointPtr closest_connection;
        for (auto &begin_point : entry_node_list) {
          float new_distance = end_point->DistanceSquared(begin_point);
          if (new_distance < min_distance and i != j) {
            min_distance = new_distance;
            closest_connection = begin_point;
          }
          ++j;
        }
        cg::Vector3D end_point_vector = end_point->GetForwardVector();
        cg::Vector3D relative_vector = closest_connection->GetLocation() - end_point->GetLocation();
        relative_vector = relative_vector.MakeUnitVector();
        float relative_dot = cg::Math::Dot(end_point_vector, relative_vector);
        if (relative_dot < LANE_CHANGE_ANGULAR_THRESHOLD) {
          uint count = LANE_CHANGE_LOOK_AHEAD;
          while (count > 0) {
            closest_connection = closest_connection->GetNextWaypoint()[0];
            --count;
          }
        }
        end_point->SetNextWaypoint({closest_connection});
      }
      ++i;
    }

    // Linking lane change connections.
    for (auto &simple_waypoint:dense_topology) {
      FindAndLinkLaneChange(simple_waypoint);
    }

  }

  SimpleWaypointPtr InMemoryMap::GetWaypoint(const cg::Location &location) const {

    SimpleWaypointPtr closest_waypoint;
    float min_distance = INFINITE_DISTANCE;
    for (auto &simple_waypoint : dense_topology) {
      float current_distance = simple_waypoint->DistanceSquared(location);
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

    WaypointPtr current_waypoint = waypoint->GetWaypoint();
    uint section_id = current_waypoint->GetSectionId();
    uint road_id = current_waypoint->GetRoadId();
    int lane_id = current_waypoint->GetLaneId();

    // Find the road by its id.
    if (road_to_waypoint.find(road_id) != road_to_waypoint.end()) {

      SectionWaypointMap &section_map = road_to_waypoint.at(road_id);
      // Find the section of the road by its id.
      if (section_map.find(section_id) != section_map.end()) {

        LaneWaypointMap &lane_map = section_map.at(section_id);
        // Find the lane of the section by its id.
        if (lane_map.find(lane_id) != lane_map.end()) {

          // Place the waypoint in the lane's list.
          lane_map.at(lane_id).push_back(waypoint);
        } else {

          // Create a new list to hold waypoints for the lane.
          std::vector<SimpleWaypointPtr> lane_waypoint_list;
          lane_waypoint_list.push_back(waypoint);
          // Insert the new list into the lane map.
          lane_map.insert({lane_id, lane_waypoint_list});
        }
      } else {

        std::vector<SimpleWaypointPtr> lane_waypoint_list;
        lane_waypoint_list.push_back(waypoint);
        // Create a new lane map to hold the waypoint list.
        LaneWaypointMap lane_map;
        lane_map.insert({lane_id, lane_waypoint_list});
        // Insert the new lane map into the corresponding section map.
        section_map.insert({section_id, lane_map});
      }
    } else {

      std::vector<SimpleWaypointPtr> lane_waypoint_list;
      lane_waypoint_list.push_back(waypoint);
      LaneWaypointMap lane_map;
      lane_map.insert({lane_id, lane_waypoint_list});
      // Create a new section map to hold the lane map.
      SectionWaypointMap section_map;
      section_map.insert({section_id, lane_map});
      // Insert the new section map into the corresponding road.
      road_to_waypoint.insert({road_id, section_map});
    }
  }

  void InMemoryMap::LinkLaneChangePoint(
      SimpleWaypointPtr reference_waypoint,
      WaypointPtr neighbor_waypoint,
      int side) {

    if (neighbor_waypoint != nullptr) {
      uint neighbour_road_id = neighbor_waypoint->GetRoadId();
      uint neighbour_section_id = neighbor_waypoint->GetSectionId();
      int neighbour_lane_id = neighbor_waypoint->GetLaneId();

      // Find waypoint samples in dense topology corresponding to the
      // geo ids of the neighbor waypoint found using Carla's server call.
      if (road_to_waypoint.find(neighbour_road_id) != road_to_waypoint.end() &&
          (road_to_waypoint[neighbour_road_id].find(neighbour_section_id)
          != road_to_waypoint[neighbour_road_id].end()) &&
          (road_to_waypoint[neighbour_road_id][neighbour_section_id].find(neighbour_lane_id)
          != road_to_waypoint[neighbour_road_id][neighbour_section_id].end())) {

        std::vector<SimpleWaypointPtr> waypoints_to_left =
            road_to_waypoint[neighbour_road_id][neighbour_section_id][neighbour_lane_id];

        // Find the nearest sample to the neighbor waypoint to be used as a
        // local cache representative to be linked for indicating a lane change
        // connection.
        if (waypoints_to_left.size() > 0) {
          SimpleWaypointPtr nearest_waypoint = waypoints_to_left[0];
          float smallest_left_distance = INFINITE_DISTANCE;
          for (SimpleWaypointPtr &left_wp : waypoints_to_left) {
            if (reference_waypoint->DistanceSquared(left_wp) < smallest_left_distance) {
              smallest_left_distance = reference_waypoint->DistanceSquared(left_wp);
              nearest_waypoint = left_wp;
            }
          }

          // Place appropriate lane change link.
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

    WaypointPtr raw_waypoint = reference_waypoint->GetWaypoint();
    uint8_t lane_change = static_cast<uint8_t>(raw_waypoint->GetLaneChange());
    uint8_t change_right = static_cast<uint8_t>(carla::road::element::LaneMarking::LaneChange::Right);
    uint8_t change_left = static_cast<uint8_t>(carla::road::element::LaneMarking::LaneChange::Left);

    if ((lane_change & change_right) > 0) {
      WaypointPtr right_waypoint =  raw_waypoint->GetRight();
      LinkLaneChangePoint(reference_waypoint, right_waypoint, +1);
    }
    if ((lane_change & change_left) > 0) {
      WaypointPtr left_waypoint =  raw_waypoint->GetLeft();
      LinkLaneChangePoint(reference_waypoint, left_waypoint, -1);
    }

  }
}
