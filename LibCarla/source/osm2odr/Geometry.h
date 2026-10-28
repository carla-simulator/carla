// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cmath>
#include <optional>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace twinmodel {
namespace geom {

  namespace bg = boost::geometry;
  namespace bgi = boost::geometry::index;

  using Point = bg::model::d2::point_xy<double>;
  using LineString = bg::model::linestring<Point>;
  using Ring = bg::model::ring<Point, true, true>;
  using Polygon = bg::model::polygon<Point, true, true>;
  using MultiPolygon = bg::model::multi_polygon<Polygon>;
  using Box = bg::model::box<Point>;

  // A vertex carrying the elevation the reference line was sampled at; the
  // planar algorithms ignore z, the OpenDRIVE elevation profile reads it back.
  struct Point3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
  };

  using Line3 = std::vector<Point3>;

  inline Point To2D(const Point3 &p) {
    return Point(p.x, p.y);
  }

  inline LineString To2D(const Line3 &line) {
    LineString out;
    out.reserve(line.size());
    for (const auto &p : line) {
      bg::append(out, To2D(p));
    }
    return out;
  }

  inline double Length(const LineString &line) {
    return bg::length(line);
  }

  /// Distance along @a line of the projection of @a p, shapely's LineString.project.
  double Project(const LineString &line, const Point &p);

  /// Point at distance @a s along @a line, shapely's LineString.interpolate.
  Point Interpolate(const LineString &line, double s);

  /// Heading of @a line at distance @a s, in radians.
  double HeadingAt(const LineString &line, double s);

  /// The piece of @a line between distances @a from and @a to, shapely's
  /// substring: endpoints are inserted so the cut is exact.
  LineString Substring(const LineString &line, double from, double to);

  /// Offset @a line sideways by @a offset metres, positive to the left.
  LineString OffsetLine(const LineString &line, double offset);

  /// Build polygons from a set of noded edges, shapely's polygonize. Used for
  /// junction outlines, where the input is the union of the incoming kerbs.
  std::vector<Polygon> Polygonize(const std::vector<LineString> &edges);

  MultiPolygon UnaryUnion(const std::vector<Polygon> &polygons);

  /// bg::is_valid plus a correction pass, shapely's make_valid.
  Polygon MakeValid(const Polygon &polygon);

  inline Point Centroid(const Polygon &polygon) {
    Point c(0.0, 0.0);
    bg::centroid(polygon, c);
    return c;
  }

  inline double Hypot(const Point &a, const Point &b) {
    return std::hypot(bg::get<0>(b) - bg::get<0>(a), bg::get<1>(b) - bg::get<1>(a));
  }

  /// Signed angle in (-pi, pi] that turns heading @a from into heading @a to.
  inline double NormalizeAngle(double angle) {
    while (angle <= -M_PI) angle += 2.0 * M_PI;
    while (angle > M_PI) angle -= 2.0 * M_PI;
    return angle;
  }

} // namespace geom
} // namespace twinmodel
