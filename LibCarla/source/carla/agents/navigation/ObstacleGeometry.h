// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/multi_point.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <array>
#include <vector>

namespace carla {
namespace agents {
namespace navigation {
namespace detail {

using ObstaclePoint = boost::geometry::model::d2::point_xy<double>;
using ObstaclePolygon = boost::geometry::model::polygon<ObstaclePoint>;

/// Paired right/left route samples form a perimeter in opposite directions.
inline ObstaclePolygon BuildRoutePolygon(
    const std::vector<std::array<double, 3>> &samples) {
  ObstaclePolygon polygon;
  for (size_t i = 0u; i < samples.size(); i += 2u) {
    boost::geometry::append(polygon.outer(), ObstaclePoint(samples[i][0], samples[i][1]));
  }
  for (size_t i = samples.size(); i > 0u; i -= 2u) {
    boost::geometry::append(polygon.outer(), ObstaclePoint(samples[i - 1u][0], samples[i - 1u][1]));
  }
  boost::geometry::correct(polygon);
  return polygon;
}

/// The projected box vertices are unordered and may include duplicate points.
inline ObstaclePolygon ProjectBoundingBox(const std::array<geom::Location, 8> &vertices) {
  boost::geometry::model::multi_point<ObstaclePoint> points;
  for (const auto &vertex : vertices) {
    boost::geometry::append(points, ObstaclePoint(vertex.x, vertex.y));
  }
  ObstaclePolygon polygon;
  boost::geometry::convex_hull(points, polygon);
  return polygon;
}

} // namespace detail
} // namespace navigation
} // namespace agents
} // namespace carla
