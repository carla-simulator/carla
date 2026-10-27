// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "LaneGraph.h"

#include <algorithm>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <tuple>

#include "Profiles.h"

namespace twinmodel {

  namespace {

    struct WaySegment {
      long long way_id = 0;
      std::string highway;
      Tags tags;
      std::vector<long long> nodes;
    };

    /// Endpoint of a road at a junction, with the heading pointing away from
    /// the junction; the connecting roads are built by pairing these up.
    /// A road built from one stretch of a way, with the OSM nodes its ends sit
    /// on so stretches of consecutive ways can be chained into one road.
    struct PieceEnds {
      long long first = 0;
      long long last = 0;
    };

    struct JunctionLeg {
      std::string road_id;
      ContactPoint contact = ContactPoint::Start;
      geom::Point point;
      double heading = 0.0;
      long long node = 0;
    };


    /// Disjoint sets over OSM node ids, for the junction clustering.
    class UnionFind {
    public:

      void Add(long long id) {
        _parent.emplace(id, id);
      }

      long long Find(long long id) {
        auto it = _parent.find(id);
        if (it == _parent.end()) {
          _parent.emplace(id, id);
          return id;
        }
        while (it->second != id) {
          id = it->second;
          it = _parent.find(id);
        }
        return id;
      }

      void Join(long long a, long long b) {
        const long long ra = Find(a);
        const long long rb = Find(b);
        if (ra != rb) {
          _parent[ra] = rb;
        }
      }

    private:

      std::map<long long, long long> _parent;
    };

    std::string MakeId(const char *prefix, std::size_t index) {
      std::ostringstream out;
      out << prefix << index;
      return out.str();
    }

    /// Lanes outward from the reference line: negative ids run forward, the
    /// positive ones backward, which is OpenDRIVE's convention for RHT.
    void FillLanes(Road &road, const LaneProfile &profile) {
      int id = -1;
      for (int i = 0; i < profile.forward_lanes; ++i, --id) {
        Lane lane;
        lane.id = id;
        lane.type = LaneType::Driving;
        lane.width = profile.lane_width;
        lane.direction = Direction::Forward;
        lane.speed_limit = profile.speed_limit;
        road.lanes.push_back(lane);
      }
      if (profile.shoulder.has_value() && profile.forward_lanes > 0) {
        Lane lane;
        lane.id = id--;
        lane.type = LaneType::Shoulder;
        lane.width = *profile.shoulder;
        lane.direction = Direction::Forward;
        road.lanes.push_back(lane);
      }
      if (profile.sidewalk_width.has_value() && profile.forward_lanes > 0) {
        Lane lane;
        lane.id = id--;
        lane.type = LaneType::Sidewalk;
        lane.width = *profile.sidewalk_width;
        lane.direction = Direction::Both;
        road.lanes.push_back(lane);
      }

      id = 1;
      for (int i = 0; i < profile.backward_lanes; ++i, ++id) {
        Lane lane;
        lane.id = id;
        lane.type = LaneType::Driving;
        lane.width = profile.lane_width;
        lane.direction = Direction::Backward;
        lane.speed_limit = profile.speed_limit;
        road.lanes.push_back(lane);
      }
      if (profile.shoulder.has_value() && profile.backward_lanes > 0) {
        Lane lane;
        lane.id = id++;
        lane.type = LaneType::Shoulder;
        lane.width = *profile.shoulder;
        lane.direction = Direction::Backward;
        road.lanes.push_back(lane);
      }
      if (profile.sidewalk_width.has_value() && profile.backward_lanes > 0) {
        Lane lane;
        lane.id = id++;
        lane.type = LaneType::Sidewalk;
        lane.width = *profile.sidewalk_width;
        lane.direction = Direction::Both;
        road.lanes.push_back(lane);
      }

      if (profile.backward_lanes > 0 && profile.forward_lanes > 0) {
        Marking centre;
        centre.kind = MarkingKind::Broken;
        centre.color = MarkingColor::White;
        road.center_marking = centre;
      }
    }


    /// Lanes of @a road that arrive at the junction touching the given end,
    /// and the lanes that leave from it. At the start of a road the backward
    /// lanes (positive ids) arrive and the forward ones (negative) leave.
    void SplitLanesAtContact(const Road &road, ContactPoint contact,
                             std::vector<int> &arriving, std::vector<int> &leaving) {
      for (const Lane &lane : road.lanes) {
        if (lane.type != LaneType::Driving) {
          continue;
        }
        const bool forward = lane.id < 0;
        const bool arrives = (contact == ContactPoint::Start) ? !forward : forward;
        (arrives ? arriving : leaving).push_back(lane.id);
      }
    }

    /// Cubic Hermite from @a from (heading into the junction) to @a to
    /// (heading out of it), sampled about every metre.
    geom::Line3 ConnectingGeometry(const geom::Point &from, double heading_in,
                                   const geom::Point &to, double heading_out, double z) {
      const double dx = geom::bg::get<0>(to) - geom::bg::get<0>(from);
      const double dy = geom::bg::get<1>(to) - geom::bg::get<1>(from);
      const double span = std::max(1.0, std::hypot(dx, dy));
      const double tangent = span * 0.65;
      const double x0 = geom::bg::get<0>(from);
      const double y0 = geom::bg::get<1>(from);
      const double x1 = geom::bg::get<0>(to);
      const double y1 = geom::bg::get<1>(to);
      const double mx0 = std::cos(heading_in) * tangent;
      const double my0 = std::sin(heading_in) * tangent;
      const double mx1 = std::cos(heading_out) * tangent;
      const double my1 = std::sin(heading_out) * tangent;

      const int steps = std::max(4, static_cast<int>(span));
      geom::Line3 line;
      line.reserve(static_cast<std::size_t>(steps) + 1u);
      for (int i = 0; i <= steps; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(steps);
        const double t2 = t * t;
        const double t3 = t2 * t;
        line.push_back(geom::Point3{
            (2.0 * t3 - 3.0 * t2 + 1.0) * x0 + (t3 - 2.0 * t2 + t) * mx0 +
            (-2.0 * t3 + 3.0 * t2) * x1 + (t3 - t2) * mx1,
            (2.0 * t3 - 3.0 * t2 + 1.0) * y0 + (t3 - 2.0 * t2 + t) * my0 +
            (-2.0 * t3 + 3.0 * t2) * y1 + (t3 - t2) * my1,
            z});
      }
      return line;
    }

    double LineLength(const geom::Line3 &line) {
      double total = 0.0;
      for (std::size_t i = 1u; i < line.size(); ++i) {
        total += std::hypot(line[i].x - line[i - 1u].x, line[i].y - line[i - 1u].y);
      }
      return total;
    }

    /// Trim @a length metres off the given end of a reference line, leaving the
    /// junction box free for the connecting roads.
    void TrimEnd(geom::Line3 &line, double length, bool from_start) {
      if (line.size() < 2u || length <= 0.0) {
        return;
      }
      const double total = LineLength(line);
      if (total <= length * 2.0 + 0.5) {
        return;
      }
      geom::LineString flat = geom::To2D(line);
      const double from = from_start ? length : 0.0;
      const double to = from_start ? total : total - length;
      const geom::LineString cut = geom::Substring(flat, from, to);
      geom::Line3 trimmed;
      trimmed.reserve(cut.size());
      const double z = line.front().z;
      for (const auto &p : cut) {
        trimmed.push_back(geom::Point3{geom::bg::get<0>(p), geom::bg::get<1>(p), z});
      }
      line = std::move(trimmed);
    }


    /// Keep the stretch of every road that lies inside @a box and drop the
    /// roads that fall outside it, with the junctions left dangling removed.
    void ClipToBox(TwinModelData &model, const geom::Box &box) {
      auto inside = [&box](const geom::Point3 &p) {
        return p.x >= box.min_corner().x() && p.x <= box.max_corner().x() &&
               p.y >= box.min_corner().y() && p.y <= box.max_corner().y();
      };
      std::vector<Road> kept;
      std::set<std::string> kept_ids;
      // a connector is only useful while both the road it leaves and the one
      // it joins survive the clip
      std::set<std::string> survivors;
      for (const Road &road : model.roads) {
        // the same test the keep loop below applies: a road needs two points
        // inside the box to leave a drivable stretch behind
        std::size_t first = road.reference_line.size();
        std::size_t last = 0u;
        for (std::size_t i = 0u; i < road.reference_line.size(); ++i) {
          if (inside(road.reference_line[i])) {
            first = std::min(first, i);
            last = std::max(last, i);
          }
        }
        if (first < road.reference_line.size() && last > first) {
          survivors.insert(road.id);
        }
      }
      for (Road &road : model.roads) {
        if (road.junction_id.has_value()) {
          const bool from_ok = !road.predecessor.has_value() ||
              survivors.count(road.predecessor->id) != 0u;
          const bool to_ok = !road.successor.has_value() ||
              survivors.count(road.successor->id) != 0u;
          if (!from_ok || !to_ok) {
            continue;
          }
        }
        std::size_t first = road.reference_line.size();
        std::size_t last = 0u;
        for (std::size_t i = 0u; i < road.reference_line.size(); ++i) {
          if (inside(road.reference_line[i])) {
            first = std::min(first, i);
            last = std::max(last, i);
          }
        }
        if (first >= road.reference_line.size() || last <= first) {
          continue;
        }
        if (first != 0u || last + 1u != road.reference_line.size()) {
          geom::Line3 trimmed(road.reference_line.begin() + static_cast<long>(first),
                              road.reference_line.begin() + static_cast<long>(last) + 1);
          road.reference_line = std::move(trimmed);
          // a trimmed road no longer reaches whatever it used to link to
          road.predecessor.reset();
          road.successor.reset();
        }
        kept_ids.insert(road.id);
        kept.push_back(std::move(road));
      }
      model.roads = std::move(kept);

      std::vector<Junction> junctions;
      for (Junction &junction : model.junctions) {
        std::vector<Connection> connections;
        for (Connection &connection : junction.connections) {
          if (kept_ids.count(connection.incoming_road) != 0u &&
              kept_ids.count(connection.connecting_road) != 0u) {
            connections.push_back(std::move(connection));
          }
        }
        if (connections.empty()) {
          continue;
        }
        junction.connections = std::move(connections);
        junctions.push_back(std::move(junction));
      }
      model.junctions = std::move(junctions);

      // drop links that point at a road or junction the clip removed
      std::set<std::string> junction_ids;
      for (const Junction &junction : model.junctions) {
        junction_ids.insert(junction.id);
      }
      for (Road &road : model.roads) {
        auto prune = [&](std::optional<RoadLink> &link) {
          if (!link.has_value()) {
            return;
          }
          const bool alive = (link->element == LinkElement::Road)
                           ? kept_ids.count(link->id) != 0u
                           : junction_ids.count(link->id) != 0u;
          if (!alive) {
            link.reset();
          }
        };
        prune(road.predecessor);
        prune(road.successor);
        if (road.junction_id.has_value() && junction_ids.count(*road.junction_id) == 0u) {
          road.junction_id.reset();
        }
      }
    }

  } // namespace

  TwinModelData BuildLaneGraph(
      const OsmData &osm,
      const LocalFrame &frame,
      const std::string &name,
      const LaneGraphOptions &options) {

    TwinModelData model;
    model.name = name;
    model.origin_lat = frame.GetOriginLatitude();
    model.origin_lon = frame.GetOriginLongitude();

    // 1. the drivable ways, and how often each node is used by one
    std::vector<WaySegment> drivable;
    std::map<long long, int> node_use;
    for (const OsmWay &way : osm.ways) {
      const std::string *highway = FindTag(way.tags, "highway");
      if (highway == nullptr || !IsDrivable(*highway)) {
        continue;
      }
      WaySegment segment;
      segment.way_id = way.id;
      segment.highway = *highway;
      segment.tags = way.tags;
      segment.nodes = way.nodes;
      // degree counts incident segments, so an interior node scores 2 and a
      // way end 1; two ways meeting end to end stay degree 2 and get chained
      for (std::size_t i = 0u; i < way.nodes.size(); ++i) {
        const bool interior = (i > 0u) && (i + 1u < way.nodes.size());
        node_use[way.nodes[i]] += interior ? 2 : 1;
      }
      drivable.push_back(std::move(segment));
    }

    // only a node where three or more segments meet is an intersection
    std::set<long long> junction_nodes;
    for (const auto &entry : node_use) {
      if (entry.second >= 3) {
        junction_nodes.insert(entry.first);
      }
    }

    auto node_point = [&](long long id, geom::Point &out) {
      const auto it = osm.nodes.find(id);
      if (it == osm.nodes.end()) {
        return false;
      }
      double x = 0.0;
      double y = 0.0;
      frame.ToLocal(it->second.lon, it->second.lat, x, y);
      out = geom::Point(x, y);
      return true;
    };

    // Cluster intersection nodes: two join when the chain of degree-2 nodes
    // between them is shorter than cluster_m and they are closer than that.
    UnionFind clusters;
    std::map<long long, geom::Box> extent_of;
    for (long long node : junction_nodes) {
      clusters.Add(node);
      geom::Point point;
      if (node_point(node, point)) {
        extent_of[node] = geom::Box(point, point);
      }
    }
    for (const WaySegment &way : drivable) {
      long long previous = 0;
      bool have_previous = false;
      double chain = 0.0;
      geom::Point last_point;
      bool have_last = false;
      for (long long node : way.nodes) {
        geom::Point point;
        if (!node_point(node, point)) {
          continue;
        }
        if (have_last) {
          chain += geom::Hypot(last_point, point);
        }
        last_point = point;
        have_last = true;
        if (junction_nodes.count(node) == 0u) {
          continue;
        }
        if (have_previous && chain <= options.cluster_m) {
          geom::Point previous_point;
          if (node_point(previous, previous_point) &&
              geom::Hypot(previous_point, point) <= options.cluster_m) {
            // bound the merged cluster: without this a chain of short hops
            // daisy-chains separate intersections into one junction
            const long long ra = clusters.Find(previous);
            const long long rb = clusters.Find(node);
            if (ra != rb) {
              geom::Box merged = extent_of[ra];
              geom::bg::expand(merged, extent_of[rb]);
              const double width = merged.max_corner().x() - merged.min_corner().x();
              const double height = merged.max_corner().y() - merged.min_corner().y();
              if (std::max(width, height) <= options.cluster_m * 0.75) {
                clusters.Join(previous, node);
                extent_of[clusters.Find(previous)] = merged;
              }
            }
          }
        }
        previous = node;
        have_previous = true;
        chain = 0.0;
      }
    }

    // 2. cut each way at its junction nodes and project to model space
    std::map<long long, std::string> junction_of_node;
    std::vector<JunctionLeg> legs;
    std::map<std::string, PieceEnds> ends_of_road;
    std::size_t road_index = 0u;

    auto project = [&](long long node_id, geom::Point3 &out) {
      const auto it = osm.nodes.find(node_id);
      if (it == osm.nodes.end()) {
        return false;
      }
      double x = 0.0;
      double y = 0.0;
      frame.ToLocal(it->second.lon, it->second.lat, x, y);
      out = geom::Point3{x, y, 0.0};
      return true;
    };

    for (const WaySegment &way : drivable) {
      // Split where the way crosses a junction cluster, not at every shared
      // node: a run of nodes inside one cluster is a single crossing, so the
      // stretches between clusters stay one road.
      std::vector<std::vector<long long>> pieces;
      std::vector<long long> current;
      long long open_cluster = 0;
      bool inside_cluster = false;
      for (std::size_t i = 0u; i < way.nodes.size(); ++i) {
        const long long node = way.nodes[i];
        const bool is_junction = junction_nodes.count(node) != 0u;
        const long long cluster = is_junction ? clusters.Find(node) : 0;

        if (is_junction && (!inside_cluster || cluster != open_cluster)) {
          // leaving the previous stretch: close it on this node
          current.push_back(node);
          if (current.size() >= 2u) {
            pieces.push_back(current);
          }
          current.clear();
          current.push_back(node);
          open_cluster = cluster;
          inside_cluster = true;
          continue;
        }
        if (is_junction && inside_cluster && cluster == open_cluster) {
          // still inside the same junction: drop the intermediate node and
          // restart the next stretch from it
          current.clear();
          current.push_back(node);
          continue;
        }
        inside_cluster = false;
        current.push_back(node);
      }
      if (current.size() >= 2u) {
        pieces.push_back(current);
      }

      const LaneProfile profile = ResolveProfile(way.highway, way.tags);
      for (const std::vector<long long> &piece : pieces) {
        Road road;
        road.id = MakeId("road_", road_index++);
        road.highway = way.highway;
        road.osm_way_ids.push_back(way.way_id);
        road.tags = way.tags;
        const std::string *way_name = FindTag(way.tags, "name");
        if (way_name != nullptr) {
          road.name = *way_name;
        }
        for (long long node : piece) {
          geom::Point3 point;
          if (project(node, point)) {
            road.reference_line.push_back(point);
          }
        }
        if (road.reference_line.size() < 2u ||
            LineLength(road.reference_line) < options.min_road_length) {
          --road_index;
          continue;
        }
        FillLanes(road, profile);

        // remember the ends that sit on a junction node before trimming
        const long long first_node = piece.front();
        const long long last_node = piece.back();
        const bool start_on_junction = junction_nodes.count(first_node) != 0u;
        const bool end_on_junction = junction_nodes.count(last_node) != 0u;

        if (start_on_junction) {
          TrimEnd(road.reference_line, options.junction_radius, true);
        }
        if (end_on_junction) {
          TrimEnd(road.reference_line, options.junction_radius, false);
        }

        const geom::LineString flat = geom::To2D(road.reference_line);
        const double total = geom::Length(flat);
        if (start_on_junction) {
          JunctionLeg leg;
          leg.road_id = road.id;
          leg.contact = ContactPoint::Start;
          leg.point = flat.front();
          leg.heading = geom::NormalizeAngle(geom::HeadingAt(flat, 0.0) + M_PI);
          leg.node = first_node;
          legs.push_back(leg);
          junction_of_node.emplace(first_node, std::string());
        }
        if (end_on_junction) {
          JunctionLeg leg;
          leg.road_id = road.id;
          leg.contact = ContactPoint::End;
          leg.point = flat.back();
          leg.heading = geom::HeadingAt(flat, total);
          leg.node = last_node;
          legs.push_back(leg);
          junction_of_node.emplace(last_node, std::string());
        }
        ends_of_road[road.id] = PieceEnds{first_node, last_node};
        model.roads.push_back(std::move(road));
      }
    }

    // Chain roads that meet at a node no junction owns: a street mapped as
    // several consecutive OSM ways becomes one road, which is what keeps the
    // leg count at a junction down.
    {
      std::map<long long, std::vector<std::string>> at_node;
      for (const auto &entry : ends_of_road) {
        at_node[entry.second.first].push_back(entry.first);
        if (entry.second.last != entry.second.first) {
          at_node[entry.second.last].push_back(entry.first);
        }
      }
      bool merged_any = true;
      while (merged_any) {
        merged_any = false;
        for (const auto &entry : at_node) {
          const long long node = entry.first;
          if (junction_nodes.count(node) != 0u || entry.second.size() != 2u) {
            continue;
          }
          Road *a = nullptr;
          Road *b = nullptr;
          for (Road &road : model.roads) {
            if (road.junction_id.has_value()) continue;
            if (road.id == entry.second[0]) a = &road;
            if (road.id == entry.second[1]) b = &road;
          }
          if (a == nullptr || b == nullptr || a == b) {
            continue;
          }
          // a street keeps its identity across an OSM way break even when the
          // class is tagged differently either side; the lane layout is what
          // has to line up for the merged road to keep one lane section
          if (a->lanes.size() != b->lanes.size()) {
            continue;
          }
          bool same_layout = true;
          for (std::size_t k = 0u; k < a->lanes.size(); ++k) {
            if (a->lanes[k].id != b->lanes[k].id || a->lanes[k].type != b->lanes[k].type ||
                std::abs(a->lanes[k].width - b->lanes[k].width) > 0.01) {
              same_layout = false;
              break;
            }
          }
          if (!same_layout) {
            continue;
          }
          PieceEnds &ea = ends_of_road[a->id];
          PieceEnds &eb = ends_of_road[b->id];
          // orient both so a ends at the node and b starts from it
          auto flip_legs = [&legs](const std::string &road_id) {
            for (JunctionLeg &leg : legs) {
              if (leg.road_id == road_id) {
                leg.contact = (leg.contact == ContactPoint::Start) ? ContactPoint::End
                                                                  : ContactPoint::Start;
              }
            }
          };
          if (ea.first == node) {
            std::reverse(a->reference_line.begin(), a->reference_line.end());
            std::swap(ea.first, ea.last);
            flip_legs(a->id);
          }
          if (eb.last == node) {
            std::reverse(b->reference_line.begin(), b->reference_line.end());
            std::swap(eb.first, eb.last);
            flip_legs(b->id);
          }
          if (ea.last != node || eb.first != node) {
            continue;
          }
          for (std::size_t k = 1u; k < b->reference_line.size(); ++k) {
            a->reference_line.push_back(b->reference_line[k]);
          }
          ea.last = eb.last;
          for (auto &list : at_node) {
            for (std::string &id : list.second) {
              if (id == b->id) {
                id = a->id;
              }
            }
          }
          const std::string dead = b->id;
          // b's far end becomes a's end; its legs move with it
          for (JunctionLeg &leg : legs) {
            if (leg.road_id == dead) {
              leg.road_id = a->id;
              leg.contact = ContactPoint::End;
            }
          }
          ends_of_road.erase(dead);
          model.roads.erase(std::remove_if(model.roads.begin(), model.roads.end(),
              [&dead](const Road &r) { return r.id == dead; }), model.roads.end());
          merged_any = true;
          break;
        }
      }
    }

    // 3. one junction per shared node, with its legs attached
    // A road with both ends in one cluster is internal to that junction: the
    // junction's own connectors span it, and leaving it in strands them on a
    // stub. lanegraph.py calls this junction_internal_m.
    {
      const double internal_limit = 2.0 * options.cluster_m;
      std::map<std::string, std::vector<std::size_t>> legs_of_road;
      for (std::size_t i = 0u; i < legs.size(); ++i) {
        legs_of_road[legs[i].road_id].push_back(i);
      }
      std::set<std::string> internal;
      for (const auto &entry : legs_of_road) {
        if (entry.second.size() < 2u) {
          continue;
        }
        const long long root = clusters.Find(legs[entry.second.front()].node);
        bool same = true;
        for (std::size_t i : entry.second) {
          same = same && (clusters.Find(legs[i].node) == root);
        }
        if (!same) {
          continue;
        }
        const Road *road = model.FindRoad(entry.first);
        if (road != nullptr && LineLength(road->reference_line) <= internal_limit) {
          internal.insert(entry.first);
        }
      }
      if (!internal.empty()) {
        model.roads.erase(std::remove_if(model.roads.begin(), model.roads.end(),
            [&internal](const Road &r) { return internal.count(r.id) != 0u; }),
            model.roads.end());
        std::vector<JunctionLeg> surviving;
        for (JunctionLeg &leg : legs) {
          if (internal.count(leg.road_id) == 0u) {
            surviving.push_back(leg);
          }
        }
        legs = std::move(surviving);
      }
    }

    std::map<long long, std::vector<std::size_t>> legs_of_node;
    for (std::size_t i = 0u; i < legs.size(); ++i) {
      legs_of_node[clusters.Find(legs[i].node)].push_back(i);
    }

    std::size_t junction_index = 0u;
    std::vector<Road> connecting_roads;
    for (const auto &entry : legs_of_node) {
      if (entry.second.size() < 3u) {
        // two legs only: the roads meet end to end, link them directly
        if (entry.second.size() == 2u) {
          JunctionLeg &a = legs[entry.second[0]];
          JunctionLeg &b = legs[entry.second[1]];
          Road *road_a = nullptr;
          Road *road_b = nullptr;
          for (Road &road : model.roads) {
            if (road.id == a.road_id) road_a = &road;
            if (road.id == b.road_id) road_b = &road;
          }
          if (road_a != nullptr && road_b != nullptr) {
            RoadLink link_a{LinkElement::Road, b.road_id, b.contact};
            RoadLink link_b{LinkElement::Road, a.road_id, a.contact};
            (a.contact == ContactPoint::Start ? road_a->predecessor : road_a->successor) = link_a;
            (b.contact == ContactPoint::Start ? road_b->predecessor : road_b->successor) = link_b;
            // Two roads meeting end to end keep their lane numbering when the
            // contacts are opposite, and mirror it when they are the same.
            const bool mirrored = (a.contact == b.contact);
            for (Lane &lane : road_a->lanes) {
              const int target = mirrored ? -lane.id : lane.id;
              bool exists = false;
              for (const Lane &other : road_b->lanes) {
                exists = exists || (other.id == target);
              }
              if (!exists) {
                continue;
              }
              (a.contact == ContactPoint::Start ? lane.predecessor_lane
                                                : lane.successor_lane) = target;
            }
            for (Lane &lane : road_b->lanes) {
              const int target = mirrored ? -lane.id : lane.id;
              bool exists = false;
              for (const Lane &other : road_a->lanes) {
                exists = exists || (other.id == target);
              }
              if (!exists) {
                continue;
              }
              (b.contact == ContactPoint::Start ? lane.predecessor_lane
                                                : lane.successor_lane) = target;
            }
          }
        }
        continue;
      }

      Junction junction;
      junction.id = MakeId("junction_", junction_index++);
      // keyed on both ends: a road touching the junction at its start and at
      // its end offers two different movements to the same target
      std::map<std::tuple<std::string, ContactPoint, std::string, ContactPoint>,
               std::pair<std::string, double>> best_movement;
      for (long long node : junction_nodes) {
        if (clusters.Find(node) == entry.first) {
          junction.osm_node_ids.push_back(node);
        }
      }

      for (std::size_t i : entry.second) {
        for (std::size_t j : entry.second) {
          if (i == j) {
            continue;
          }
          const JunctionLeg &in = legs[i];
          const JunctionLeg &out = legs[j];
          // a leg never connects back to itself through the junction
          // both headings point away from the junction, so a U-turn back down
          // the same leg reads as ~0 and a straight-through as ~pi
          const double turn = std::abs(geom::NormalizeAngle(out.heading - in.heading));
          if (turn < (M_PI - options.uturn_deg * M_PI / 180.0)) {
            continue;
          }
          const Road *road_in = model.FindRoad(in.road_id);
          const Road *road_out = model.FindRoad(out.road_id);
          if (road_in == nullptr || road_out == nullptr) {
            continue;
          }
          std::vector<int> arriving;
          std::vector<int> leaving;
          std::vector<int> ignored;
          SplitLanesAtContact(*road_in, in.contact, arriving, ignored);
          ignored.clear();
          SplitLanesAtContact(*road_out, out.contact, ignored, leaving);
          if (arriving.empty() || leaving.empty()) {
            continue;
          }

          Road connecting;
          connecting.id = MakeId("road_", road_index++);
          connecting.junction_id = junction.id;
          connecting.highway = road_in->highway;
          connecting.reference_line = ConnectingGeometry(
              in.point, geom::NormalizeAngle(in.heading + M_PI),
              out.point, out.heading,
              road_in->reference_line.empty() ? 0.0 : road_in->reference_line.front().z);

          const std::size_t width = std::min(arriving.size(), leaving.size());
          for (std::size_t k = 0u; k < width; ++k) {
            Lane lane;
            lane.id = -static_cast<int>(k) - 1;
            lane.type = LaneType::Driving;
            lane.width = road_in->lanes.empty() ? 3.5 : road_in->lanes.front().width;
            lane.direction = Direction::Forward;
            lane.predecessor_lane = arriving[k];
            lane.successor_lane = leaving[k];
            connecting.lanes.push_back(lane);
          }
          connecting.predecessor = RoadLink{LinkElement::Road, in.road_id, in.contact};
          connecting.successor = RoadLink{LinkElement::Road, out.road_id, out.contact};

          // one connector per movement: a cluster can hold several legs of the
          // same road, which would otherwise serve the same turn twice
          const std::tuple<std::string, ContactPoint, std::string, ContactPoint> movement{
              in.road_id, in.contact, out.road_id, out.contact};
          const double length = LineLength(connecting.reference_line);
          const auto seen = best_movement.find(movement);
          if (seen != best_movement.end()) {
            if (seen->second.second <= length) {
              --road_index;
              continue;
            }
            const std::string superseded = seen->second.first;
            connecting_roads.erase(
                std::remove_if(connecting_roads.begin(), connecting_roads.end(),
                    [&superseded](const Road &r) { return r.id == superseded; }),
                connecting_roads.end());
            junction.connections.erase(
                std::remove_if(junction.connections.begin(), junction.connections.end(),
                    [&superseded](const Connection &c) {
                      return c.connecting_road == superseded;
                    }),
                junction.connections.end());
          }
          best_movement[movement] = std::make_pair(connecting.id, length);

          Connection connection;
          connection.id = MakeId("connection_", junction.connections.size());
          connection.incoming_road = in.road_id;
          connection.connecting_road = connecting.id;
          connection.contact_point = ContactPoint::Start;
          for (std::size_t k = 0u; k < width; ++k) {
            connection.lane_links.push_back(LaneLink{arriving[k], -static_cast<int>(k) - 1});
          }
          junction.connections.push_back(std::move(connection));
          connecting_roads.push_back(std::move(connecting));
        }
      }

      for (std::size_t i : entry.second) {
        for (Road &road : model.roads) {
          if (road.id != legs[i].road_id) {
            continue;
          }
          RoadLink link{LinkElement::Junction, junction.id, std::nullopt};
          if (legs[i].contact == ContactPoint::Start) {
            road.predecessor = link;
          } else {
            road.successor = link;
          }
        }
      }
      model.junctions.push_back(std::move(junction));
    }

    for (Road &road : connecting_roads) {
      model.roads.push_back(std::move(road));
    }

    if (options.clip.has_value()) {
      ClipToBox(model, *options.clip);
    }

    return model;
  }

} // namespace twinmodel
