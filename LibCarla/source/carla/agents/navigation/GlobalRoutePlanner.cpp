// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/agents/navigation/GlobalRoutePlanner.h"

#include "carla/client/Map.h"
#include "carla/client/Waypoint.h"
#include "carla/geom/Transform.h"
#include "carla/road/Lane.h"
#include "carla/road/element/LaneMarking.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace carla {
namespace agents {
namespace navigation {

namespace {

  // Mirrors agents.tools.misc.vector(): unit vector from a -> b with an
  // epsilon added to the norm to avoid division by zero (matches numpy's
  // `np.finfo(float).eps`).
  geom::Vector3D UnitVector(const geom::Location &a, const geom::Location &b) {
    geom::Vector3D v(b.x - a.x, b.y - a.y, b.z - a.z);
    const double norm = std::sqrt(static_cast<double>(v.x) * v.x
                                + static_cast<double>(v.y) * v.y
                                + static_cast<double>(v.z) * v.z)
                      + std::numeric_limits<double>::epsilon();
    return geom::Vector3D(
        static_cast<float>(v.x / norm),
        static_cast<float>(v.y / norm),
        static_cast<float>(v.z / norm));
  }

  // Bitwise-AND test on the LaneChange enum class — Python uses
  // `lane_change & carla.LaneChange.Right` and treats non-zero as truthy.
  bool LaneChangeAllows(road::element::LaneMarking::LaneChange lc,
                        road::element::LaneMarking::LaneChange flag) {
    using U = std::underlying_type_t<road::element::LaneMarking::LaneChange>;
    return (static_cast<U>(lc) & static_cast<U>(flag)) != 0;
  }

} // namespace

// -----------------------------------------------------------------------------
// ctor: build pipeline mirrors the Python __init__:
//   _build_topology -> _build_graph -> _find_loose_ends -> _lane_change_link.
// -----------------------------------------------------------------------------
GlobalRoutePlanner::GlobalRoutePlanner(SharedPtr<client::Map> map,
                                       double sampling_resolution)
  : _map(std::move(map)),
    _sampling_resolution(sampling_resolution) {
  BuildTopology();
  BuildGraph();
  FindLooseEnds();
  LaneChangeLink();
}

// -----------------------------------------------------------------------------
// graph mutators
// -----------------------------------------------------------------------------
GlobalRoutePlanner::NodeId GlobalRoutePlanner::GetOrCreateNode(const XYZKey &key) {
  auto it = _id_map.find(key);
  if (it != _id_map.end()) {
    return it->second;
  }
  const NodeId new_id = static_cast<NodeId>(_id_map.size());
  _id_map.emplace(key, new_id);
  Node node;
  node.vertex = {std::get<0>(key), std::get<1>(key), std::get<2>(key)};
  _nodes.emplace(new_id, std::move(node));
  return new_id;
}

GlobalRoutePlanner::Edge &GlobalRoutePlanner::AddEdge(NodeId n1, NodeId n2) {
  auto &edge = _edges[{n1, n2}];
  edge.src = n1;
  edge.dst = n2;
  // Track the successor only on the first insert (matches DiGraph semantics).
  auto &node = _nodes[n1];
  if (std::find(node.successors.begin(), node.successors.end(), n2)
      == node.successors.end()) {
    node.successors.push_back(n2);
  }
  return edge;
}

const GlobalRoutePlanner::Edge *
GlobalRoutePlanner::FindEdge(NodeId n1, NodeId n2) const {
  auto it = _edges.find({n1, n2});
  return it == _edges.end() ? nullptr : &it->second;
}

GlobalRoutePlanner::Edge *
GlobalRoutePlanner::FindEdge(NodeId n1, NodeId n2) {
  auto it = _edges.find({n1, n2});
  return it == _edges.end() ? nullptr : &it->second;
}

// -----------------------------------------------------------------------------
// _build_topology
// -----------------------------------------------------------------------------
void GlobalRoutePlanner::BuildTopology() {
  _topology.clear();

  for (const auto &segment : _map->GetTopology()) {
    SharedPtr<client::Waypoint> wp1 = segment.first;
    SharedPtr<client::Waypoint> wp2 = segment.second;

    const auto &l1 = wp1->GetTransform().location;
    const auto &l2 = wp2->GetTransform().location;

    // Round to integer coords to deduplicate near-identical endpoints — must
    // match `np.round([...], 0)` exactly.
    const double x1 = std::round(static_cast<double>(l1.x));
    const double y1 = std::round(static_cast<double>(l1.y));
    const double z1 = std::round(static_cast<double>(l1.z));
    const double x2 = std::round(static_cast<double>(l2.x));
    const double y2 = std::round(static_cast<double>(l2.y));
    const double z2 = std::round(static_cast<double>(l2.z));

    TopologySegment seg;
    seg.entry = wp1;
    seg.exit  = wp2;
    seg.entry_xyz = std::make_tuple(x1, y1, z1);
    seg.exit_xyz  = std::make_tuple(x2, y2, z2);

    const auto end_loc = wp2->GetTransform().location;
    if (wp1->GetTransform().location.Distance(end_loc) > _sampling_resolution) {
      auto next_wps = wp1->GetNext(_sampling_resolution);
      if (next_wps.empty()) {
        // Mirror Python: still record the (empty-path) segment.
        _topology.push_back(std::move(seg));
        continue;
      }
      auto w = next_wps[0];
      while (w && w->GetTransform().location.Distance(end_loc) > _sampling_resolution) {
        seg.path.push_back(w);
        auto step = w->GetNext(_sampling_resolution);
        if (step.empty()) {
          break;
        }
        w = step[0];
      }
    } else {
      auto next_wps = wp1->GetNext(_sampling_resolution);
      if (next_wps.empty()) {
        continue;  // skip degenerate segment
      }
      seg.path.push_back(next_wps[0]);
    }

    _topology.push_back(std::move(seg));
  }
}

// -----------------------------------------------------------------------------
// _build_graph
// -----------------------------------------------------------------------------
void GlobalRoutePlanner::BuildGraph() {
  _nodes.clear();
  _edges.clear();
  _id_map.clear();
  _road_id_to_edge.clear();

  for (const auto &segment : _topology) {
    const auto &entry_wp = segment.entry;
    const auto &exit_wp  = segment.exit;
    const bool intersection = entry_wp->IsJunction();
    const auto road_id    = entry_wp->GetRoadId();
    const auto section_id = entry_wp->GetSectionId();
    const auto lane_id    = entry_wp->GetLaneId();

    const NodeId n1 = GetOrCreateNode(segment.entry_xyz);
    const NodeId n2 = GetOrCreateNode(segment.exit_xyz);

    _road_id_to_edge[road_id][section_id][lane_id] = {n1, n2};

    const auto entry_v = entry_wp->GetTransform().rotation.GetForwardVector();
    const auto exit_v  = exit_wp->GetTransform().rotation.GetForwardVector();

    Edge &edge = AddEdge(n1, n2);
    // Python: length = len(path) + 1.
    edge.length        = static_cast<double>(segment.path.size()) + 1.0;
    edge.path          = segment.path;
    edge.entry_waypoint = entry_wp;
    edge.exit_waypoint  = exit_wp;
    edge.entry_vector   = entry_v;
    edge.exit_vector    = exit_v;
    edge.net_vector     = UnitVector(entry_wp->GetTransform().location,
                                     exit_wp->GetTransform().location);
    edge.intersection   = intersection;
    edge.type           = RoadOption::LaneFollow;
  }
}

// -----------------------------------------------------------------------------
// _find_loose_ends — synthesize terminal nodes with NEGATIVE ids for road
// segments whose exit doesn't appear as anyone's entry.
// -----------------------------------------------------------------------------
void GlobalRoutePlanner::FindLooseEnds() {
  int count_loose_ends = 0;
  const double hop = _sampling_resolution;

  for (const auto &segment : _topology) {
    const auto &end_wp = segment.exit;
    const auto road_id    = end_wp->GetRoadId();
    const auto section_id = end_wp->GetSectionId();
    const auto lane_id    = end_wp->GetLaneId();

    bool already_indexed = false;
    auto road_it = _road_id_to_edge.find(road_id);
    if (road_it != _road_id_to_edge.end()) {
      auto sec_it = road_it->second.find(section_id);
      if (sec_it != road_it->second.end()) {
        already_indexed = sec_it->second.count(lane_id) > 0;
      }
    }
    if (already_indexed) {
      continue;
    }

    ++count_loose_ends;
    const NodeId n1 = _id_map.at(segment.exit_xyz);
    const NodeId n2 = -1 * count_loose_ends;            // NEGATIVE id sentinel
    _road_id_to_edge[road_id][section_id][lane_id] = {n1, n2};

    std::vector<SharedPtr<client::Waypoint>> path;
    auto next_wps = end_wp->GetNext(hop);
    while (!next_wps.empty()
           && next_wps[0]
           && next_wps[0]->GetRoadId()    == road_id
           && next_wps[0]->GetSectionId() == section_id
           && next_wps[0]->GetLaneId()    == lane_id) {
      path.push_back(next_wps[0]);
      next_wps = next_wps[0]->GetNext(hop);
    }

    if (!path.empty()) {
      const auto &tail_loc = path.back()->GetTransform().location;
      Node terminal;
      terminal.vertex = {static_cast<double>(tail_loc.x),
                         static_cast<double>(tail_loc.y),
                         static_cast<double>(tail_loc.z)};
      _nodes.emplace(n2, std::move(terminal));

      Edge &edge = AddEdge(n1, n2);
      edge.length         = static_cast<double>(path.size()) + 1.0;
      edge.path           = path;
      edge.entry_waypoint = end_wp;
      edge.exit_waypoint  = path.back();
      edge.entry_vector   = std::nullopt;
      edge.exit_vector    = std::nullopt;
      edge.net_vector     = std::nullopt;
      edge.intersection   = end_wp->IsJunction();
      edge.type           = RoadOption::LaneFollow;
    }
  }
}

// -----------------------------------------------------------------------------
// _lane_change_link — zero-length edges from segment entry to the localized
// neighbor lane. Python only adds at most one Right and one Left per segment.
// -----------------------------------------------------------------------------
void GlobalRoutePlanner::LaneChangeLink() {
  using LaneChange = road::element::LaneMarking::LaneChange;

  for (const auto &segment : _topology) {
    bool left_found = false;
    bool right_found = false;
    if (segment.entry->IsJunction()) {
      // Python only checks `not segment['entry'].is_junction` inside the loop;
      // skipping the loop when true is equivalent and saves work.
      continue;
    }

    for (const auto &waypoint : segment.path) {
      if (left_found && right_found) {
        break;
      }

      // Right lane change.
      if (!right_found) {
        auto right_marking = waypoint->GetRightLaneMarking();
        if (right_marking
            && LaneChangeAllows(right_marking->lane_change, LaneChange::Right)) {
          auto next_wp = waypoint->GetRight();
          if (next_wp
              && next_wp->GetType() == road::Lane::LaneType::Driving
              && waypoint->GetRoadId() == next_wp->GetRoadId()) {
            auto next_segment = Localize(next_wp->GetTransform().location);
            if (next_segment.has_value()) {
              const NodeId src = _id_map.at(segment.entry_xyz);
              const NodeId dst = next_segment->first;
              Edge &edge = AddEdge(src, dst);
              edge.entry_waypoint = waypoint;
              edge.exit_waypoint  = next_wp;
              edge.intersection   = false;
              edge.exit_vector    = std::nullopt;
              edge.path.clear();
              edge.length         = 0.0;
              edge.type           = RoadOption::ChangeLaneRight;
              edge.change_waypoint = next_wp;
              right_found = true;
            }
          }
        }
      }

      // Left lane change.
      if (!left_found) {
        auto left_marking = waypoint->GetLeftLaneMarking();
        if (left_marking
            && LaneChangeAllows(left_marking->lane_change, LaneChange::Left)) {
          auto next_wp = waypoint->GetLeft();
          if (next_wp
              && next_wp->GetType() == road::Lane::LaneType::Driving
              && waypoint->GetRoadId() == next_wp->GetRoadId()) {
            auto next_segment = Localize(next_wp->GetTransform().location);
            if (next_segment.has_value()) {
              const NodeId src = _id_map.at(segment.entry_xyz);
              const NodeId dst = next_segment->first;
              Edge &edge = AddEdge(src, dst);
              edge.entry_waypoint = waypoint;
              edge.exit_waypoint  = next_wp;
              edge.intersection   = false;
              edge.exit_vector    = std::nullopt;
              edge.path.clear();
              edge.length         = 0.0;
              edge.type           = RoadOption::ChangeLaneLeft;
              edge.change_waypoint = next_wp;
              left_found = true;
            }
          }
        }
      }
    }
  }
}

// -----------------------------------------------------------------------------
// _localize
// -----------------------------------------------------------------------------
std::optional<std::pair<GlobalRoutePlanner::NodeId, GlobalRoutePlanner::NodeId>>
GlobalRoutePlanner::Localize(const geom::Location &location) const {
  auto wp = _map->GetWaypoint(location);
  if (!wp) {
    return std::nullopt;
  }
  auto road_it = _road_id_to_edge.find(wp->GetRoadId());
  if (road_it == _road_id_to_edge.end()) return std::nullopt;
  auto sec_it = road_it->second.find(wp->GetSectionId());
  if (sec_it == road_it->second.end()) return std::nullopt;
  auto lane_it = sec_it->second.find(wp->GetLaneId());
  if (lane_it == sec_it->second.end()) return std::nullopt;
  return lane_it->second;
}

// -----------------------------------------------------------------------------
// _distance_heuristic
// -----------------------------------------------------------------------------
double GlobalRoutePlanner::DistanceHeuristic(NodeId a, NodeId b) const {
  const auto &va = _nodes.at(a).vertex;
  const auto &vb = _nodes.at(b).vertex;
  const double dx = va[0] - vb[0];
  const double dy = va[1] - vb[1];
  const double dz = va[2] - vb[2];
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// -----------------------------------------------------------------------------
// _path_search — A* with euclidean heuristic on rounded XYZ vertices.
// -----------------------------------------------------------------------------
std::vector<GlobalRoutePlanner::NodeId>
GlobalRoutePlanner::PathSearch(const geom::Location &origin,
                               const geom::Location &destination) {
  auto start = Localize(origin);
  auto end   = Localize(destination);
  if (!start.has_value() || !end.has_value()) {
    return {};
  }
  const NodeId source = start->first;
  const NodeId target = end->first;

  // Standard A*: priority queue keyed on f-score = g + h.
  using Item = std::pair<double, NodeId>;
  std::priority_queue<Item, std::vector<Item>, std::greater<Item>> open;
  std::unordered_map<NodeId, double> g_score;
  std::unordered_map<NodeId, NodeId> came_from;
  std::unordered_set<NodeId> closed;

  g_score[source] = 0.0;
  open.emplace(DistanceHeuristic(source, target), source);

  while (!open.empty()) {
    auto [f, current] = open.top();
    open.pop();
    if (current == target) {
      // Reconstruct path source -> target.
      std::vector<NodeId> path;
      NodeId n = current;
      path.push_back(n);
      while (n != source) {
        auto it = came_from.find(n);
        if (it == came_from.end()) break;
        n = it->second;
        path.push_back(n);
      }
      std::reverse(path.begin(), path.end());
      // Python appends end[1] (the second node id of the destination edge).
      path.push_back(end->second);
      return path;
    }
    if (!closed.insert(current).second) {
      continue;  // stale entry
    }

    auto node_it = _nodes.find(current);
    if (node_it == _nodes.end()) continue;
    const double g_current = g_score[current];

    for (NodeId neighbor : node_it->second.successors) {
      const Edge *edge = FindEdge(current, neighbor);
      if (!edge) continue;
      const double tentative = g_current + edge->length;
      auto it = g_score.find(neighbor);
      if (it == g_score.end() || tentative < it->second) {
        g_score[neighbor] = tentative;
        came_from[neighbor] = current;
        const double f_score = tentative + DistanceHeuristic(neighbor, target);
        open.emplace(f_score, neighbor);
      }
    }
  }

  return {};  // unreachable
}

// -----------------------------------------------------------------------------
// _successive_last_intersection_edge
// -----------------------------------------------------------------------------
std::pair<std::optional<GlobalRoutePlanner::NodeId>, const GlobalRoutePlanner::Edge *>
GlobalRoutePlanner::SuccessiveLastIntersectionEdge(
    std::size_t index, const std::vector<NodeId> &route) const {
  const Edge *last_intersection_edge = nullptr;
  std::optional<NodeId> last_node;

  for (std::size_t i = index; i + 1 < route.size(); ++i) {
    const NodeId n1 = route[i];
    const NodeId n2 = route[i + 1];
    const Edge *candidate = FindEdge(n1, n2);
    if (!candidate) break;
    if (n1 == route[index]) {
      last_intersection_edge = candidate;
    }
    if (candidate->type == RoadOption::LaneFollow && candidate->intersection) {
      last_intersection_edge = candidate;
      last_node = n2;
    } else {
      break;
    }
  }
  return {last_node, last_intersection_edge};
}

// -----------------------------------------------------------------------------
// _turn_decision — uses 2-D cross-product Z (cv.x*nv.y - cv.y*nv.x).
// -----------------------------------------------------------------------------
RoadOption GlobalRoutePlanner::TurnDecision(std::size_t index,
                                            const std::vector<NodeId> &route) {
  // math.radians(35) — hard-coded to avoid depending on the non-standard M_PI.
  static constexpr double threshold = 35.0 * 3.14159265358979323846 / 180.0;
  RoadOption decision = RoadOption::Void;

  const NodeId current_node = route[index];
  const NodeId next_node    = route[index + 1];
  const Edge *next_edge     = FindEdge(current_node, next_node);
  if (!next_edge) {
    _previous_decision = RoadOption::Void;
    return RoadOption::Void;
  }

  if (index > 0) {
    const NodeId previous_node = route[index - 1];
    if (_previous_decision != RoadOption::Void
        && _intersection_end_node > 0
        && _intersection_end_node != previous_node
        && next_edge->type == RoadOption::LaneFollow
        && next_edge->intersection) {
      decision = _previous_decision;
    } else {
      _intersection_end_node = -1;
      const Edge *current_edge = FindEdge(previous_node, current_node);
      if (!current_edge) {
        _previous_decision = next_edge->type;
        return next_edge->type;
      }
      const bool calculate_turn = current_edge->type == RoadOption::LaneFollow
                                  && !current_edge->intersection
                                  && next_edge->type == RoadOption::LaneFollow
                                  && next_edge->intersection;
      if (calculate_turn) {
        auto [last_node, tail_edge] = SuccessiveLastIntersectionEdge(index, route);
        _intersection_end_node = last_node.value_or(-1);
        if (tail_edge != nullptr) {
          next_edge = tail_edge;
        }
        const auto &cv_opt = current_edge->exit_vector;
        const auto &nv_opt = next_edge->exit_vector;
        if (!cv_opt.has_value() || !nv_opt.has_value()) {
          _previous_decision = next_edge->type;
          return next_edge->type;
        }
        const auto cv = *cv_opt;
        const auto nv = *nv_opt;

        // 2-D cross product Z-component for siblings (only LANEFOLLOW
        // successors that aren't the chosen next-node).
        std::vector<double> cross_list;
        auto succ_it = _nodes.find(current_node);
        if (succ_it != _nodes.end()) {
          for (NodeId neighbor : succ_it->second.successors) {
            const Edge *select_edge = FindEdge(current_node, neighbor);
            if (!select_edge) continue;
            if (select_edge->type == RoadOption::LaneFollow
                && neighbor != route[index + 1]
                && select_edge->net_vector.has_value()) {
              const auto &sv = *select_edge->net_vector;
              cross_list.push_back(
                  static_cast<double>(cv.x) * sv.y
                  - static_cast<double>(cv.y) * sv.x);
            }
          }
        }
        const double next_cross = static_cast<double>(cv.x) * nv.y
                                  - static_cast<double>(cv.y) * nv.x;

        const double cv_len = std::sqrt(static_cast<double>(cv.x) * cv.x
                                       + static_cast<double>(cv.y) * cv.y
                                       + static_cast<double>(cv.z) * cv.z);
        const double nv_len = std::sqrt(static_cast<double>(nv.x) * nv.x
                                       + static_cast<double>(nv.y) * nv.y
                                       + static_cast<double>(nv.z) * nv.z);
        double cos_val = 0.0;
        if (cv_len > 0.0 && nv_len > 0.0) {
          cos_val = (static_cast<double>(cv.x) * nv.x
                     + static_cast<double>(cv.y) * nv.y
                     + static_cast<double>(cv.z) * nv.z) / (cv_len * nv_len);
        }
        cos_val = std::clamp(cos_val, -1.0, 1.0);
        const double deviation = std::acos(cos_val);

        if (cross_list.empty()) {
          cross_list.push_back(0.0);
        }

        if (deviation < threshold) {
          decision = RoadOption::Straight;
        } else if (next_cross < *std::min_element(cross_list.begin(), cross_list.end())) {
          decision = RoadOption::Left;
        } else if (next_cross > *std::max_element(cross_list.begin(), cross_list.end())) {
          decision = RoadOption::Right;
        } else if (next_cross < 0.0) {
          decision = RoadOption::Left;
        } else if (next_cross > 0.0) {
          decision = RoadOption::Right;
        }
      } else {
        decision = next_edge->type;
      }
    }
  } else {
    decision = next_edge->type;
  }

  _previous_decision = decision;
  return decision;
}

// -----------------------------------------------------------------------------
// _find_closest_in_list
// -----------------------------------------------------------------------------
std::size_t GlobalRoutePlanner::FindClosestInList(
    const SharedPtr<client::Waypoint> &current,
    const std::vector<SharedPtr<client::Waypoint>> &list) {
  double min_distance = std::numeric_limits<double>::infinity();
  std::size_t closest = 0;
  const auto &cur_loc = current->GetTransform().location;
  for (std::size_t i = 0; i < list.size(); ++i) {
    const double d = list[i]->GetTransform().location.Distance(cur_loc);
    if (d < min_distance) {
      min_distance = d;
      closest = i;
    }
  }
  return closest;
}

// -----------------------------------------------------------------------------
// trace_route
// -----------------------------------------------------------------------------
std::vector<GlobalRoutePlanner::RouteItem>
GlobalRoutePlanner::TraceRoute(const geom::Location &origin,
                               const geom::Location &destination) {
  std::vector<RouteItem> route_trace;
  std::vector<NodeId> route = PathSearch(origin, destination);
  if (route.size() < 2) {
    return route_trace;
  }

  SharedPtr<client::Waypoint> current_waypoint = _map->GetWaypoint(origin);
  SharedPtr<client::Waypoint> destination_waypoint = _map->GetWaypoint(destination);
  if (!current_waypoint || !destination_waypoint) {
    return route_trace;
  }

  for (std::size_t i = 0; i + 1 < route.size(); ++i) {
    const RoadOption road_option = TurnDecision(i, route);
    const Edge *edge = FindEdge(route[i], route[i + 1]);
    if (!edge) continue;

    if (edge->type != RoadOption::LaneFollow && edge->type != RoadOption::Void) {
      route_trace.emplace_back(current_waypoint, road_option);
      const auto &exit_wp = edge->exit_waypoint;
      auto road_it = _road_id_to_edge.find(exit_wp->GetRoadId());
      if (road_it == _road_id_to_edge.end()) continue;
      auto sec_it = road_it->second.find(exit_wp->GetSectionId());
      if (sec_it == road_it->second.end()) continue;
      auto lane_it = sec_it->second.find(exit_wp->GetLaneId());
      if (lane_it == sec_it->second.end()) continue;
      const auto &n_pair = lane_it->second;
      const Edge *next_edge = FindEdge(n_pair.first, n_pair.second);
      if (next_edge && !next_edge->path.empty()) {
        std::size_t closest_index = FindClosestInList(current_waypoint, next_edge->path);
        closest_index = std::min(next_edge->path.size() - 1, closest_index + 5);
        current_waypoint = next_edge->path[closest_index];
      } else if (next_edge) {
        current_waypoint = next_edge->exit_waypoint;
      }
      route_trace.emplace_back(current_waypoint, road_option);
    } else {
      // path = [entry] + path + [exit]
      std::vector<SharedPtr<client::Waypoint>> path;
      path.reserve(edge->path.size() + 2);
      path.push_back(edge->entry_waypoint);
      path.insert(path.end(), edge->path.begin(), edge->path.end());
      path.push_back(edge->exit_waypoint);

      const std::size_t closest_index = FindClosestInList(current_waypoint, path);
      bool emitted_break = false;
      for (std::size_t k = closest_index; k < path.size() && !emitted_break; ++k) {
        current_waypoint = path[k];
        route_trace.emplace_back(current_waypoint, road_option);
        const std::size_t remaining = route.size() - i;
        if (remaining <= 2
            && current_waypoint->GetTransform().location.Distance(destination)
               < 2.0 * _sampling_resolution) {
          emitted_break = true;
        } else if (remaining <= 2
                   && current_waypoint->GetRoadId()    == destination_waypoint->GetRoadId()
                   && current_waypoint->GetSectionId() == destination_waypoint->GetSectionId()
                   && current_waypoint->GetLaneId()    == destination_waypoint->GetLaneId()) {
          const std::size_t destination_index = FindClosestInList(destination_waypoint, path);
          if (closest_index > destination_index) {
            emitted_break = true;
          }
        }
      }
    }
  }

  return route_trace;
}

} // namespace navigation
} // namespace agents
} // namespace carla
