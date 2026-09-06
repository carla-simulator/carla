// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/agents/navigation/Types.h"
#include "carla/geom/Location.h"
#include "carla/geom/Vector3D.h"

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace carla {

namespace client {
  class Map;
  class Waypoint;
} // namespace client

namespace agents {
namespace navigation {

  /// C++ port of agents.navigation.global_route_planner.GlobalRoutePlanner.
  /// Builds a directed graph from carla::client::Map::GetTopology() and runs
  /// A* search (euclidean heuristic, edge weight = `length`) to produce a
  /// (waypoint, RoadOption) plan between two world locations.
  class GlobalRoutePlanner {
  public:

    using RouteItem = std::pair<SharedPtr<client::Waypoint>, RoadOption>;

    GlobalRoutePlanner(SharedPtr<client::Map> map, double sampling_resolution);

    /// Returns the list of (waypoint, RoadOption) pairs from `origin` to
    /// `destination`. Mirrors Python `trace_route`.
    std::vector<RouteItem> TraceRoute(const geom::Location &origin,
                                      const geom::Location &destination);

  private:

    using NodeId = int;
    using XYZKey = std::tuple<double, double, double>;

    struct Edge {
      NodeId src = 0;
      NodeId dst = 0;
      double length = 0.0;                                      // matches Python `length`
      std::vector<SharedPtr<client::Waypoint>> path;
      SharedPtr<client::Waypoint> entry_waypoint;
      SharedPtr<client::Waypoint> exit_waypoint;
      std::optional<geom::Vector3D> entry_vector;
      std::optional<geom::Vector3D> exit_vector;
      std::optional<geom::Vector3D> net_vector;
      bool intersection = false;
      RoadOption type = RoadOption::LaneFollow;
      SharedPtr<client::Waypoint> change_waypoint;              // lane-change links only
    };

    struct Node {
      std::array<double, 3> vertex{{0.0, 0.0, 0.0}};
      std::vector<NodeId> successors;                           // outgoing neighbors
    };

    struct TopologySegment {
      SharedPtr<client::Waypoint> entry;
      SharedPtr<client::Waypoint> exit;
      XYZKey entry_xyz;
      XYZKey exit_xyz;
      std::vector<SharedPtr<client::Waypoint>> path;
    };

    // ---- pipeline stages (mirror Python `_build_*` / `_find_loose_ends` / `_lane_change_link`)
    void BuildTopology();
    void BuildGraph();
    void FindLooseEnds();
    void LaneChangeLink();

    // ---- helpers (mirror Python `_localize`, `_path_search`, etc.)
    std::optional<std::pair<NodeId, NodeId>> Localize(const geom::Location &loc) const;
    std::vector<NodeId> PathSearch(const geom::Location &origin,
                                   const geom::Location &destination);
    double DistanceHeuristic(NodeId a, NodeId b) const;
    std::pair<std::optional<NodeId>, const Edge *>
        SuccessiveLastIntersectionEdge(std::size_t index,
                                       const std::vector<NodeId> &route) const;
    RoadOption TurnDecision(std::size_t index,
                            const std::vector<NodeId> &route);
    static std::size_t FindClosestInList(
        const SharedPtr<client::Waypoint> &current,
        const std::vector<SharedPtr<client::Waypoint>> &list);

    // ---- graph mutators
    NodeId GetOrCreateNode(const XYZKey &key);
    Edge &AddEdge(NodeId n1, NodeId n2);
    const Edge *FindEdge(NodeId n1, NodeId n2) const;
    Edge *FindEdge(NodeId n1, NodeId n2);

    // ---- members
    SharedPtr<client::Map>            _map;
    double                            _sampling_resolution;
    std::vector<TopologySegment>      _topology;
    std::unordered_map<NodeId, Node>  _nodes;
    std::map<std::pair<NodeId, NodeId>, Edge> _edges;
    std::map<XYZKey, NodeId>          _id_map;
    // road_id -> section_id -> lane_id -> (n1, n2)
    std::map<uint32_t,
        std::map<uint32_t,
            std::map<int32_t, std::pair<NodeId, NodeId>>>> _road_id_to_edge;

    // Mutable per-call-sequence state — Python keeps these as instance attrs
    // and never resets them between trace_route() calls. Preserve that.
    NodeId      _intersection_end_node = -1;
    RoadOption  _previous_decision     = RoadOption::Void;
  };

} // namespace navigation
} // namespace agents
} // namespace carla
