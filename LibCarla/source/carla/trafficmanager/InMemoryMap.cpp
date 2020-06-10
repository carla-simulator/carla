// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/InMemoryMap.h"

namespace carla {
namespace traffic_manager {


  namespace cg = carla::geom;
  using namespace constants::Map;

  using TopologyList = std::vector<std::pair<WaypointPtr, WaypointPtr>>;
  using RawNodeList = std::vector<WaypointPtr>;

  InMemoryMap::InMemoryMap(WorldMap world_map) : _world_map(world_map) {}
  InMemoryMap::~InMemoryMap() {}

  SegmentId InMemoryMap::GetSegmentId(const WaypointPtr &wp) const {
    return std::make_tuple(wp->GetRoadId(), wp->GetLaneId(), wp->GetSectionId());
  }

  SegmentId InMemoryMap::GetSegmentId(const SimpleWaypointPtr &swp) const {
    return GetSegmentId(swp->GetWaypoint());
  }

  std::vector<SimpleWaypointPtr> InMemoryMap::GetSuccessors(const SegmentId segment_id,
  const SegmentTopology &segment_topology, const SegmentMap &segment_map) {
    std::vector<SimpleWaypointPtr> result;
    if (segment_topology.find(segment_id) == segment_topology.end()) {
      return result;
    }

    for (const auto &successor_segment_id : segment_topology.at(segment_id).second) {
      if (segment_map.find(successor_segment_id) == segment_map.end()) {
        auto successors = GetSuccessors(successor_segment_id, segment_topology, segment_map);
        result.insert(result.end(), successors.begin(), successors.end());
      } else {
        result.emplace_back(segment_map.at(successor_segment_id).front());
      }
    }
    return result;
  }

  std::vector<SimpleWaypointPtr> InMemoryMap::GetPredecessors(const SegmentId segment_id,
  const SegmentTopology &segment_topology, const SegmentMap &segment_map) {
    std::vector<SimpleWaypointPtr> result;
    if (segment_topology.find(segment_id) == segment_topology.end()) {
      return result;
    }

    for (const auto &predecessor_segment_id : segment_topology.at(segment_id).first) {
      if (segment_map.find(predecessor_segment_id) == segment_map.end()) {
        auto predecessors = GetPredecessors(predecessor_segment_id, segment_topology, segment_map);
        result.insert(result.end(), predecessors.begin(), predecessors.end());
      } else {
        result.emplace_back(segment_map.at(predecessor_segment_id).back());
      }
    }
    return result;
  }

  void InMemoryMap::SetUp() {

    // 1. Building segment topology (i.e., defining set of segment predecessors and successors)
    assert(_world_map != nullptr && "No map reference found.");
    auto waypoint_topology = _world_map->GetTopology();

    SegmentTopology segment_topology;
    std::unordered_map<int64_t, std::pair<std::set<crd::RoadId>, std::set<crd::RoadId>>> std_road_connectivity;
    std::unordered_map<crd::RoadId, bool> is_real_junction;

    for (auto &connection : waypoint_topology) {
      auto &waypoint = connection.first;
      auto &successor = connection.second;

      // Setting segment predecessors and successors.
      SegmentId waypoint_segment_id = GetSegmentId(connection.first);
      SegmentId successor_segment_id = GetSegmentId(connection.second);
      using SegIdVectorPair = std::pair<std::vector<SegmentId>, std::vector<SegmentId>>;
      SegIdVectorPair &connection_first = segment_topology[waypoint_segment_id];
      SegIdVectorPair &connection_second = segment_topology[successor_segment_id];
      connection_first.second.push_back(successor_segment_id);
      connection_second.first.push_back(waypoint_segment_id);

      // From path to standard road.
      bool waypoint_is_junction = waypoint->IsJunction();
      bool successor_is_junction = successor->IsJunction();
      if (waypoint_is_junction && !successor_is_junction) {
        crd::RoadId path_id = waypoint->GetRoadId();
        int64_t std_road_id = static_cast<int64_t>(successor->GetRoadId());
        std_road_id = (successor->GetLaneId() < 0) ? -1 * std_road_id : std_road_id;

        std::set<crd::RoadId> &in_paths = std_road_connectivity[std_road_id].first;
        in_paths.insert(path_id);

        if (in_paths.size() >= 2) {
          for (auto &in_path_id: in_paths) {
            is_real_junction[in_path_id] = true;
          }
        }
      }

      // From standard road to path.
      if (!waypoint_is_junction && successor_is_junction) {
        crd::RoadId path_id = successor->GetRoadId();
        int64_t std_road_id = static_cast<int64_t>(waypoint->GetRoadId());
        std_road_id = (waypoint->GetLaneId() < 0) ? -1 * std_road_id : std_road_id;

        std::set<crd::RoadId> &out_paths = std_road_connectivity[std_road_id].second;
        out_paths.insert(path_id);

        if (out_paths.size() >= 2) {
          for (auto &out_path_id: out_paths) {
            is_real_junction[out_path_id] = true;
          }
        }
      }
    }

    // 2. Consuming the raw dense topology from cc::Map into SimpleWaypoints.
    SegmentMap segment_map;
    assert(_world_map != nullptr && "No map reference found.");
    auto raw_dense_topology = _world_map->GenerateWaypoints(MAP_RESOLUTION);
    for (auto &waypoint_ptr: raw_dense_topology) {
      segment_map[GetSegmentId(waypoint_ptr)].emplace_back(std::make_shared<SimpleWaypoint>(waypoint_ptr));
    }

    // 3. Processing waypoints.
    auto distance_squared =
    [](cg::Location l1, cg::Location l2) {
      return cg::Math::DistanceSquared(l1, l2);
    };
    auto square = [](float input) {return std::pow(input, 2);};
    auto compare_s = [](const SimpleWaypointPtr &swp1, const SimpleWaypointPtr &swp2) {
      return (swp1->GetWaypoint()->GetDistance() < swp2->GetWaypoint()->GetDistance());
    };

    GeoGridId geodesic_grid_id_counter = -1;
    for (auto &segment: segment_map) {
      auto &segment_waypoints = segment.second;

      // Generating geodesic grid ids.
      ++geodesic_grid_id_counter;

      // Ordering waypoints according to road direction.
      std::sort(segment_waypoints.begin(), segment_waypoints.end(), compare_s);
      auto lane_id = segment_waypoints.front()->GetWaypoint()->GetLaneId();
      if (lane_id > 0) {
        std::reverse(segment_waypoints.begin(), segment_waypoints.end());
      }

      // Placing intra-segment connections.
      cg::Location grid_edge_location = segment_waypoints.front()->GetLocation();
      for (std::size_t i = 0; i < segment_waypoints.size() - 1; ++i) {

        // Assigning grid id.
        if (distance_squared(grid_edge_location, segment_waypoints.at(i)->GetLocation()) >
        square(MAX_GEODESIC_GRID_LENGTH)) {
          ++geodesic_grid_id_counter;
          grid_edge_location = segment_waypoints.at(i)->GetLocation();
        }
        segment_waypoints.at(i)->SetGeodesicGridId(geodesic_grid_id_counter);

        segment_waypoints.at(i)->SetNextWaypoint({segment_waypoints.at(i + 1)});
        segment_waypoints.at(i + 1)->SetPreviousWaypoint({segment_waypoints.at(i)});
      }
      segment_waypoints.back()->SetGeodesicGridId(geodesic_grid_id_counter);

      // Adding simple waypoints to processed dense topology.
      for (auto swp: segment_waypoints) {
        // Checking whether the waypoint is a real junction.
        auto road_id = swp->GetWaypoint()->GetRoadId();
        if (swp->GetWaypoint()->IsJunction() && !is_real_junction.count(road_id)) {
          swp->SetIsJunction(false);
        } else {
          swp->SetIsJunction(swp->GetWaypoint()->IsJunction());
        }

        dense_topology.push_back(swp);
      }
    }

    // Localizing waypoints into grids.
    for (auto &simple_waypoint: dense_topology) {
      if (simple_waypoint != nullptr) {
        const cg::Location loc = simple_waypoint->GetLocation();
        Point3D point(loc.x, loc.y, loc.z);
        rtree.insert(std::make_pair(point, simple_waypoint));
      }
    }

    // Placing inter-segment connections.
    for (auto &segment : segment_map) {
      SegmentId segment_id = segment.first;
      auto &segment_waypoints = segment.second;

      auto successors = GetSuccessors(segment_id, segment_topology, segment_map);
      auto predecessors = GetPredecessors(segment_id, segment_topology, segment_map);

      segment_waypoints.front()->SetPreviousWaypoint(predecessors);
      segment_waypoints.back()->SetNextWaypoint(successors);
    }

    // Linking lane change connections.
    for (auto &simple_waypoint:dense_topology) {
      if (!simple_waypoint->CheckJunction()) {
        FindAndLinkLaneChange(simple_waypoint);
      }
    }

    // Linking any unconnected segments.
    for (auto &swp: dense_topology) {
      if (swp->GetNextWaypoint().empty()) {
        auto neighbour = swp->GetRightWaypoint();
        if (!neighbour) {
          neighbour = swp->GetLeftWaypoint();
        }

        if (neighbour) {
          swp->SetNextWaypoint(neighbour->GetNextWaypoint());
          for (auto next_waypoint : neighbour->GetNextWaypoint()) {
            next_waypoint->SetPreviousWaypoint({swp});
          }
        }
      }
    }
  }

  SimpleWaypointPtr InMemoryMap::GetWaypoint(const cg::Location loc) const {

    Point3D query_point(loc.x, loc.y, loc.z);
    std::vector<SpatialTreeEntry> result_1;

    rtree.query(bgi::nearest(query_point, 1), std::back_inserter(result_1));
    SpatialTreeEntry &closest_entry = result_1.front();
    SimpleWaypointPtr &closest_point = closest_entry.second;

    return closest_point;
  }

  std::vector<SimpleWaypointPtr> InMemoryMap::GetDenseTopology() const {
    return dense_topology;
  }

  void InMemoryMap::FindAndLinkLaneChange(SimpleWaypointPtr reference_waypoint) {

    const WaypointPtr raw_waypoint = reference_waypoint->GetWaypoint();
    const crd::element::LaneMarking::LaneChange lane_change = raw_waypoint->GetLaneChange();

    /// Cheack for transits
    switch(lane_change)
    {
      /// Left transit way point present only
      case crd::element::LaneMarking::LaneChange::Left:
      {
        const WaypointPtr left_waypoint = raw_waypoint->GetLeft();
        if (left_waypoint != nullptr &&
        left_waypoint->GetType() == crd::Lane::LaneType::Driving &&
        (left_waypoint->GetLaneId() * raw_waypoint->GetLaneId() > 0)) {

          SimpleWaypointPtr closest_simple_waypoint = GetWaypoint(left_waypoint->GetTransform().location);
          reference_waypoint->SetLeftWaypoint(closest_simple_waypoint);
        }
      }
      break;

      /// Right transit way point present only
      case crd::element::LaneMarking::LaneChange::Right:
      {
	    const WaypointPtr right_waypoint = raw_waypoint->GetRight();
	    if(right_waypoint != nullptr &&
	    right_waypoint->GetType() == crd::Lane::LaneType::Driving &&
	    (right_waypoint->GetLaneId() * raw_waypoint->GetLaneId() > 0)) {

	      SimpleWaypointPtr closest_simple_waypoint = GetWaypoint(right_waypoint->GetTransform().location);
	      reference_waypoint->SetRightWaypoint(closest_simple_waypoint);
	    }
      }
      break;

      /// Both left and right transit present
      case crd::element::LaneMarking::LaneChange::Both:
      {
        /// Right transit way point
        const WaypointPtr right_waypoint = raw_waypoint->GetRight();
        if (right_waypoint != nullptr &&
        right_waypoint->GetType() == crd::Lane::LaneType::Driving &&
        (right_waypoint->GetLaneId() * raw_waypoint->GetLaneId() > 0)) {

          SimpleWaypointPtr closest_simple_waypointR = GetWaypoint(right_waypoint->GetTransform().location);
          reference_waypoint->SetRightWaypoint(closest_simple_waypointR);
        }

        /// Left transit way point
        const WaypointPtr left_waypoint = raw_waypoint->GetLeft();
        if (left_waypoint != nullptr &&
        left_waypoint->GetType() == crd::Lane::LaneType::Driving &&
        (left_waypoint->GetLaneId() * raw_waypoint->GetLaneId() > 0)) {

          SimpleWaypointPtr closest_simple_waypointL = GetWaypoint(left_waypoint->GetTransform().location);
          reference_waypoint->SetLeftWaypoint(closest_simple_waypointL);
        }
      }
      break;

      /// For no transit waypoint (left or right)
      default: break;
    }
  }

  std::string InMemoryMap::GetMapName() {
    assert(_world_map != nullptr && "No map reference found.");
    return _world_map->GetName();
  }

} // namespace traffic_manager
} // namespace carla
