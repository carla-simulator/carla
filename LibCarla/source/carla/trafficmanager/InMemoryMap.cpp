// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "InMemoryMap.h"

namespace carla {
namespace traffic_manager {

namespace MapConstants {

  static const float INFINITE_DISTANCE = std::numeric_limits<float>::max();
  static const float GRID_SIZE = 4.0f;
  static const float PED_GRID_SIZE = 10.0f;
  static const float MAX_GEODESIC_GRID_LENGTH = 20.0f;
} // namespace MapConstants

  namespace cg = carla::geom;
  using namespace MapConstants;

  InMemoryMap::InMemoryMap(RawNodeList _raw_dense_topology) {
    raw_dense_topology = _raw_dense_topology;
  }
  InMemoryMap::~InMemoryMap() {}

  void InMemoryMap::SetUp() {

    NodeList entry_node_list;
    NodeList exit_node_list;

    auto distance_squared =
        [](cg::Location l1, cg::Location l2) {
          return cg::Math::DistanceSquared(l1, l2);
        };
    auto square = [](float input) {return std::pow(input, 2);};

    // Consuming the raw dense topology from cc::Map into SimpleWaypoints.
    std::map<std::pair<crd::RoadId, crd::LaneId>, std::vector<SimpleWaypointPtr>> segment_map;
    for (auto& waypoint_ptr: raw_dense_topology) {
      auto road_id = waypoint_ptr->GetRoadId();
      auto lane_id = waypoint_ptr->GetLaneId();
      if (segment_map.find({road_id, lane_id}) != segment_map.end()) {
        segment_map.at({road_id, lane_id}).push_back(std::make_shared<SimpleWaypoint>(waypoint_ptr));
      } else {
        segment_map.insert({{road_id, lane_id}, {std::make_shared<SimpleWaypoint>(waypoint_ptr)}});
      }
    }

    auto compare_s = [] (const SimpleWaypointPtr& swp1, const SimpleWaypointPtr& swp2) {
      return (swp1->GetWaypoint()->GetDistance() < swp2->GetWaypoint()->GetDistance());
    };

    GeoGridId geodesic_grid_id_counter = -1;
    for (auto& segment: segment_map) {

      // Generating geodesic grid ids.
      ++geodesic_grid_id_counter;

      // Ordering waypoints to be consecutive.
      auto& segment_waypoints = segment.second;
      std::sort(segment_waypoints.begin(), segment_waypoints.end(), compare_s);

      if (segment_waypoints.front()->DistanceSquared(segment_waypoints.back()) > square(0.1f)) {

        SimpleWaypointPtr first_point = segment_waypoints.at(0);
        SimpleWaypointPtr second_point = segment_waypoints.at(1);
        cg::Vector3D first_to_second = second_point->GetLocation() - first_point->GetLocation();
        first_to_second = first_to_second.MakeUnitVector();
        cg::Vector3D first_heading = first_point->GetForwardVector();

        if (cg::Math::Dot(first_heading, first_to_second) < 0.0f) {
          std::reverse(segment_waypoints.begin(), segment_waypoints.end());
        }

        // Registering segment end points.
        entry_node_list.push_back(segment_waypoints.front());
        exit_node_list.push_back(segment_waypoints.back());

        // Placing intra-segment connections.
        cg::Location grid_edge_location = segment_waypoints.front()->GetLocation();
        for (uint64_t i=0; i< segment_waypoints.size() -1; ++i) {

          // Assigning grid id.
          if (distance_squared(grid_edge_location, segment_waypoints.at(i)->GetLocation()) >
              square(MAX_GEODESIC_GRID_LENGTH)) {
            ++geodesic_grid_id_counter;
            grid_edge_location = segment_waypoints.at(i)->GetLocation();
          }
          segment_waypoints.at(i)->SetGeodesicGridId(geodesic_grid_id_counter);

          segment_waypoints.at(i)->SetNextWaypoint({segment_waypoints.at(i+1)});
          segment_waypoints.at(i+1)->SetPreviousWaypoint({segment_waypoints.at(i)});
        }
        segment_waypoints.back()->SetGeodesicGridId(geodesic_grid_id_counter);

        // Adding to processed dense topology.
        for (auto swp: segment_waypoints) {
          dense_topology.push_back(swp);
        }
      }
    }

    // Localizing waypoints into grids.
    for (auto &simple_waypoint: dense_topology) {
      if (simple_waypoint != nullptr) {
        const cg::Location loc = simple_waypoint->GetLocation();
        const std::string grid_key = MakeGridKey(MakeGridId(loc.x, loc.y, true));
        if (waypoint_grid.find(grid_key) == waypoint_grid.end()) {
          waypoint_grid.insert({grid_key, {simple_waypoint}});
        } else {
          waypoint_grid.at(grid_key).insert(simple_waypoint);
        }
        const std::string ped_grid_key = MakeGridKey(MakeGridId(loc.x, loc.y, false));
        if (ped_waypoint_grid.find(ped_grid_key) == ped_waypoint_grid.end()) {
          ped_waypoint_grid.insert({ped_grid_key, {simple_waypoint}});
        } else {
          ped_waypoint_grid.at(ped_grid_key).insert(simple_waypoint);
        }
      }
    }

    // Linking segments.
    uint64_t i = 0u, j = 0u;
    for (SimpleWaypointPtr end_point : exit_node_list) {
      j = 0u;
      for (SimpleWaypointPtr begin_point: entry_node_list) {
        if ((end_point->DistanceSquared(begin_point) < square(2.0f)) && (i != j)) {
          end_point->SetNextWaypoint({begin_point});
          begin_point->SetPreviousWaypoint({end_point});
        }
      ++j;
      }
      ++i;
    }

    // Linking lane change connections.
    for (auto &simple_waypoint:dense_topology) {
      if (!simple_waypoint->CheckJunction()) {
        FindAndLinkLaneChange(simple_waypoint);
      }
    }

    // Linking any unconnected segments.
    for (auto& swp: dense_topology) {
      if (swp->GetNextWaypoint().size() == 0) {
        SimpleWaypointPtr nearest_sample = GetWaypointInVicinity(swp->GetLocation()
            + cg::Location(swp->GetForwardVector() * 0.2f));
        swp->SetNextWaypoint({nearest_sample});
        nearest_sample->SetPreviousWaypoint({swp});
      }
    }

    MakeGeodesiGridCenters();
  }

  std::pair<int, int> InMemoryMap::MakeGridId (float x, float y, bool vehicle_or_pedestrian) {
    if (vehicle_or_pedestrian) {
      return {static_cast<int>(std::floor(x/GRID_SIZE)), static_cast<int>(std::floor(y/GRID_SIZE))};
    } else {
      return {static_cast<int>(std::floor(x/PED_GRID_SIZE)), static_cast<int>(std::floor(y/PED_GRID_SIZE))};
    }
  }

  std::string InMemoryMap::MakeGridKey (std::pair<int , int> grid_key) {
    return std::to_string(grid_key.first) + std::to_string(grid_key.second);
  }

  SimpleWaypointPtr InMemoryMap::GetWaypointInVicinity(cg::Location location) {

    const std::pair<int, int> grid_ids = MakeGridId(location.x, location.y, true);
    SimpleWaypointPtr closest_waypoint = nullptr;
    float closest_distance = INFINITE_DISTANCE;

    // Search all surrounding grids for closest waypoint.
    for (int i = -1; i <= 1; ++i) {
      for (int j = -1; j <= 1; ++j) {

        const std::string grid_key = MakeGridKey({grid_ids.first + i, grid_ids.second + j});
        if (waypoint_grid.find(grid_key) != waypoint_grid.end()) {

          const auto& waypoint_set = waypoint_grid.at(grid_key);
          if (closest_waypoint == nullptr) {
            closest_waypoint = *waypoint_set.begin();
          }

          for (auto &simple_waypoint: waypoint_set) {

            if (simple_waypoint->DistanceSquared(location) < std::pow(closest_distance, 2)) {
              closest_waypoint = simple_waypoint;
              closest_distance = simple_waypoint->DistanceSquared(location);
            }
          }
        }
      }
    }

    // Return the closest waypoint in the surrounding grids
    // only if it is in the same horizontal plane as the requested location.
    if (closest_waypoint != nullptr &&
        std::abs(closest_waypoint->GetLocation().z - location.z) > 1.0) { // Account for constant.

      closest_waypoint = nullptr;
    }

    return closest_waypoint;
  }

  SimpleWaypointPtr InMemoryMap::GetPedWaypoint(cg::Location location) {

    const std::pair<int, int> grid_ids = MakeGridId(location.x, location.y, false);
    SimpleWaypointPtr closest_waypoint = nullptr;
    float closest_distance = INFINITE_DISTANCE;

    // Search all surrounding grids for closest waypoint.
    for (int i = -1; i <= 1; ++i) {
      for (int j = -1; j <= 1; ++j) {

        const std::string grid_key = MakeGridKey({grid_ids.first + i, grid_ids.second + j});
        if (ped_waypoint_grid.find(grid_key) != ped_waypoint_grid.end()) {

          const auto& waypoint_set = ped_waypoint_grid.at(grid_key);
          if (closest_waypoint == nullptr) {
            closest_waypoint = *waypoint_set.begin();
          }

          for (auto &simple_waypoint: waypoint_set) {

            if (simple_waypoint->DistanceSquared(location) < std::pow(closest_distance, 2)) {
              closest_waypoint = simple_waypoint;
              closest_distance = simple_waypoint->DistanceSquared(location);
            }
          }
        }
      }
    }

    return closest_waypoint;
  }

  SimpleWaypointPtr InMemoryMap::GetWaypoint(const cg::Location &location) const {

    SimpleWaypointPtr closest_waypoint;
    float min_distance = INFINITE_DISTANCE;
    for (auto &simple_waypoint : dense_topology) {
      const float current_distance = simple_waypoint->DistanceSquared(location);
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

  void InMemoryMap::FindAndLinkLaneChange(SimpleWaypointPtr reference_waypoint) {

    const WaypointPtr raw_waypoint = reference_waypoint->GetWaypoint();
    const crd::element::LaneMarking::LaneChange lane_change = raw_waypoint->GetLaneChange();
    const auto change_right = crd::element::LaneMarking::LaneChange::Right;
    const auto change_left = crd::element::LaneMarking::LaneChange::Left;
    const auto change_both = crd::element::LaneMarking::LaneChange::Both;

    try {
      if (lane_change == change_right || lane_change == change_both) {

        const WaypointPtr right_waypoint =  raw_waypoint->GetRight();
        if (right_waypoint != nullptr &&
            right_waypoint->GetType() == crd::Lane::LaneType::Driving &&
            (right_waypoint->GetLaneId() * raw_waypoint->GetLaneId() > 0)) {

          SimpleWaypointPtr closest_simple_waypoint = GetWaypointInVicinity(right_waypoint->GetTransform().location);
          if (closest_simple_waypoint == nullptr) {
            closest_simple_waypoint = GetWaypoint(right_waypoint->GetTransform().location);
          }
          reference_waypoint->SetRightWaypoint(closest_simple_waypoint);
        }
      }

      if (lane_change == change_left || lane_change == change_both) {

        const WaypointPtr left_waypoint =  raw_waypoint->GetLeft();
        if (left_waypoint != nullptr &&
            left_waypoint->GetType() == crd::Lane::LaneType::Driving &&
            (left_waypoint->GetLaneId() * raw_waypoint->GetLaneId() > 0)) {

          SimpleWaypointPtr closest_simple_waypoint = GetWaypointInVicinity(left_waypoint->GetTransform().location);
          if (closest_simple_waypoint == nullptr) {
            closest_simple_waypoint = GetWaypoint(left_waypoint->GetTransform().location);
          }
          reference_waypoint->SetLeftWaypoint(closest_simple_waypoint);
        }
      }
    } catch (const std::invalid_argument &e) {
      cg::Location loc = reference_waypoint->GetLocation();
      carla::log_info(
          "Unable to link lane change connection at: "
          + std::to_string(loc.x) + " "
          + std::to_string(loc.y) + " "
          + std::to_string(loc.z));
    }
  }

  void InMemoryMap::MakeGeodesiGridCenters() {
    for (auto& swp: dense_topology) {
      GeoGridId ggid = swp->CheckJunction()? swp->GetJunctionId(): swp->GetGeodesicGridId();
      if (geodesic_grid_center.find(ggid) == geodesic_grid_center.end()) {
        geodesic_grid_center.insert({ggid, swp->GetLocation()});
      } else {
        cg::Location& grid_loc = geodesic_grid_center.at(ggid);
        grid_loc = (grid_loc + swp->GetLocation())/2;
      }
    }
  }

  cg::Location InMemoryMap::GetGeodesicGridCenter(GeoGridId ggid) {
    cg::Location grid_center;
    if (geodesic_grid_center.find(ggid) != geodesic_grid_center.end()) {
      grid_center = geodesic_grid_center.at(ggid);
    } else {
      grid_center = cg::Location();
    }
    return grid_center;
  }

} // namespace traffic_manager
} // namespace carla
