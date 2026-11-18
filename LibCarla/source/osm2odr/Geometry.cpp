// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Geometry.h"

#include <algorithm>
#include <map>
#include <set>

#include <boost/geometry/algorithms/correct.hpp>

namespace twinmodel {
namespace geom {

  namespace {

    // Vertices closer than this are the same node when the planar graph is
    // built; OSM coordinates reach us already snapped to centimetres.
    constexpr double kNodeEpsilon = 1e-6;

    struct NodeKey {
      long long x;
      long long y;
      bool operator<(const NodeKey &o) const {
        return (x != o.x) ? (x < o.x) : (y < o.y);
      }
    };

    NodeKey MakeKey(const Point &p) {
      return NodeKey{
          static_cast<long long>(std::llround(bg::get<0>(p) / kNodeEpsilon)),
          static_cast<long long>(std::llround(bg::get<1>(p) / kNodeEpsilon))};
    }

  } // namespace

  double Project(const LineString &line, const Point &p) {
    if (line.size() < 2u) {
      return 0.0;
    }
    double best_distance = std::numeric_limits<double>::max();
    double best_s = 0.0;
    double travelled = 0.0;
    for (std::size_t i = 1u; i < line.size(); ++i) {
      const Point &a = line[i - 1u];
      const Point &b = line[i];
      const double dx = bg::get<0>(b) - bg::get<0>(a);
      const double dy = bg::get<1>(b) - bg::get<1>(a);
      const double segment = std::hypot(dx, dy);
      if (segment > 0.0) {
        double t = ((bg::get<0>(p) - bg::get<0>(a)) * dx +
                    (bg::get<1>(p) - bg::get<1>(a)) * dy) / (segment * segment);
        t = std::clamp(t, 0.0, 1.0);
        const Point on(bg::get<0>(a) + t * dx, bg::get<1>(a) + t * dy);
        const double distance = Hypot(on, p);
        if (distance < best_distance) {
          best_distance = distance;
          best_s = travelled + t * segment;
        }
      }
      travelled += segment;
    }
    return best_s;
  }

  Point Interpolate(const LineString &line, double s) {
    if (line.empty()) {
      return Point(0.0, 0.0);
    }
    if (s <= 0.0) {
      return line.front();
    }
    double travelled = 0.0;
    for (std::size_t i = 1u; i < line.size(); ++i) {
      const double segment = Hypot(line[i - 1u], line[i]);
      if (travelled + segment >= s) {
        const double t = (segment > 0.0) ? (s - travelled) / segment : 0.0;
        return Point(
            bg::get<0>(line[i - 1u]) + t * (bg::get<0>(line[i]) - bg::get<0>(line[i - 1u])),
            bg::get<1>(line[i - 1u]) + t * (bg::get<1>(line[i]) - bg::get<1>(line[i - 1u])));
      }
      travelled += segment;
    }
    return line.back();
  }

  double HeadingAt(const LineString &line, double s) {
    if (line.size() < 2u) {
      return 0.0;
    }
    double travelled = 0.0;
    for (std::size_t i = 1u; i < line.size(); ++i) {
      const double segment = Hypot(line[i - 1u], line[i]);
      if (travelled + segment >= s || i + 1u == line.size()) {
        return std::atan2(bg::get<1>(line[i]) - bg::get<1>(line[i - 1u]),
                          bg::get<0>(line[i]) - bg::get<0>(line[i - 1u]));
      }
      travelled += segment;
    }
    return 0.0;
  }

  LineString Substring(const LineString &line, double from, double to) {
    LineString out;
    if (line.size() < 2u) {
      return out;
    }
    const double total = Length(line);
    from = std::clamp(from, 0.0, total);
    to = std::clamp(to, 0.0, total);
    if (to <= from) {
      return out;
    }
    bg::append(out, Interpolate(line, from));
    double travelled = 0.0;
    for (std::size_t i = 1u; i < line.size(); ++i) {
      const double segment = Hypot(line[i - 1u], line[i]);
      const double at_end = travelled + segment;
      if (at_end > from && at_end < to) {
        bg::append(out, line[i]);
      }
      travelled = at_end;
    }
    bg::append(out, Interpolate(line, to));
    return out;
  }

  LineString OffsetLine(const LineString &line, double offset) {
    LineString out;
    if (line.size() < 2u) {
      return out;
    }
    if (std::abs(offset) < 1e-12) {
      return line;
    }
    // Per-vertex normal from the mean of the adjacent segment normals, scaled
    // by 1/cos(half turn) so the offset keeps its width through a corner.
    for (std::size_t i = 0u; i < line.size(); ++i) {
      const std::size_t prev = (i == 0u) ? 0u : i - 1u;
      const std::size_t next = (i + 1u == line.size()) ? i : i + 1u;
      const double hx_in = bg::get<0>(line[i]) - bg::get<0>(line[prev]);
      const double hy_in = bg::get<1>(line[i]) - bg::get<1>(line[prev]);
      const double hx_out = bg::get<0>(line[next]) - bg::get<0>(line[i]);
      const double hy_out = bg::get<1>(line[next]) - bg::get<1>(line[i]);
      const double a_in = std::atan2(hy_in, hx_in);
      const double a_out = std::atan2(hy_out, hx_out);
      const double heading = (i == 0u) ? a_out
                           : (i + 1u == line.size()) ? a_in
                           : a_in + 0.5 * NormalizeAngle(a_out - a_in);
      const double half_turn = (i == 0u || i + 1u == line.size())
                             ? 0.0 : 0.5 * NormalizeAngle(a_out - a_in);
      const double scale = std::max(0.25, std::cos(half_turn));
      bg::append(out, Point(
          bg::get<0>(line[i]) - std::sin(heading) * offset / scale,
          bg::get<1>(line[i]) + std::cos(heading) * offset / scale));
    }
    return out;
  }

  MultiPolygon UnaryUnion(const std::vector<Polygon> &polygons) {
    MultiPolygon out;
    for (const Polygon &polygon : polygons) {
      Polygon corrected = polygon;
      bg::correct(corrected);
      if (!bg::is_valid(corrected)) {
        corrected = MakeValid(corrected);
      }
      MultiPolygon merged;
      bg::union_(out, corrected, merged);
      out = std::move(merged);
    }
    return out;
  }

  Polygon MakeValid(const Polygon &polygon) {
    Polygon out = polygon;
    bg::correct(out);
    if (bg::is_valid(out)) {
      return out;
    }
    // A zero-width buffer is the standard GEOS repair and is what shapely's
    // make_valid falls back to for self-touching rings.
    MultiPolygon buffered;
    bg::buffer(out, buffered,
               bg::strategy::buffer::distance_symmetric<double>(0.0),
               bg::strategy::buffer::side_straight(),
               bg::strategy::buffer::join_miter(),
               bg::strategy::buffer::end_flat(),
               bg::strategy::buffer::point_square());
    if (!buffered.empty()) {
      std::size_t best = 0u;
      double best_area = -1.0;
      for (std::size_t i = 0u; i < buffered.size(); ++i) {
        const double area = bg::area(buffered[i]);
        if (area > best_area) {
          best_area = area;
          best = i;
        }
      }
      return buffered[best];
    }
    return out;
  }

  std::vector<Polygon> Polygonize(const std::vector<LineString> &edges) {
    // Planar face tracing: at each node the half-edges are sorted by bearing,
    // and a face is walked by always taking the next half-edge clockwise.
    struct HalfEdge {
      std::size_t from = 0u;
      std::size_t to = 0u;
      double bearing = 0.0;
      bool used = false;
    };

    std::map<NodeKey, std::size_t> node_ids;
    std::vector<Point> nodes;
    auto node_of = [&](const Point &p) {
      const NodeKey key = MakeKey(p);
      auto it = node_ids.find(key);
      if (it != node_ids.end()) {
        return it->second;
      }
      const std::size_t id = nodes.size();
      nodes.push_back(p);
      node_ids.emplace(key, id);
      return id;
    };

    std::vector<HalfEdge> half_edges;
    for (const LineString &edge : edges) {
      for (std::size_t i = 1u; i < edge.size(); ++i) {
        const std::size_t a = node_of(edge[i - 1u]);
        const std::size_t b = node_of(edge[i]);
        if (a == b) {
          continue;
        }
        const double bearing = std::atan2(bg::get<1>(nodes[b]) - bg::get<1>(nodes[a]),
                                          bg::get<0>(nodes[b]) - bg::get<0>(nodes[a]));
        half_edges.push_back(HalfEdge{a, b, bearing, false});
        half_edges.push_back(HalfEdge{b, a, NormalizeAngle(bearing + M_PI), false});
      }
    }

    std::vector<std::vector<std::size_t>> outgoing(nodes.size());
    for (std::size_t i = 0u; i < half_edges.size(); ++i) {
      outgoing[half_edges[i].from].push_back(i);
    }
    for (auto &list : outgoing) {
      std::sort(list.begin(), list.end(), [&](std::size_t a, std::size_t b) {
        return half_edges[a].bearing < half_edges[b].bearing;
      });
    }

    std::vector<Polygon> faces;
    for (std::size_t start = 0u; start < half_edges.size(); ++start) {
      if (half_edges[start].used) {
        continue;
      }
      Ring ring;
      std::size_t current = start;
      bool closed = false;
      for (std::size_t guard = 0u; guard < half_edges.size() + 1u; ++guard) {
        half_edges[current].used = true;
        bg::append(ring, nodes[half_edges[current].from]);
        // the reverse half-edge, then one step clockwise around its target
        const std::size_t node = half_edges[current].to;
        const double back = NormalizeAngle(half_edges[current].bearing + M_PI);
        const auto &list = outgoing[node];
        if (list.empty()) {
          break;
        }
        std::size_t pick = list.front();
        double best = 10.0;
        for (std::size_t candidate : list) {
          double turn = NormalizeAngle(half_edges[candidate].bearing - back);
          if (turn <= 0.0) {
            turn += 2.0 * M_PI;
          }
          if (turn < best) {
            best = turn;
            pick = candidate;
          }
        }
        current = pick;
        if (current == start) {
          closed = true;
          break;
        }
      }
      if (!closed || ring.size() < 3u) {
        continue;
      }
      bg::append(ring, ring.front());
      Polygon face;
      face.outer() = ring;
      bg::correct(face);
      // the outer face of the arrangement comes out with the whole arrangement
      // inside it, and is dropped by its negative signed area before correction
      if (bg::area(face) > 0.0 && bg::is_valid(face)) {
        faces.push_back(face);
      }
    }
    return faces;
  }

} // namespace geom
} // namespace twinmodel
