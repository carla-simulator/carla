// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/geom/Vector3D.h"

#include <utility>
#include <cmath>

namespace carla {
namespace geom {

  class Math {
  public:

    static constexpr auto pi() {
      return 3.14159265358979323846264338327950288;
    }

    static constexpr auto pi_double() {
      return 2.0 * pi();
    }

    static constexpr auto pi_half() {
      return 0.5 * pi();
    }

    static constexpr auto to_radiants() {
      return 180.0 / pi();
    }

    static double clamp(
        const double &a,
        const double &min,
        const double &max) {
      return std::min(std::max(a, min), max);
    }

    static double clamp01(double a) {
      return clamp(a, 0.0, 1.0);
    }

    template <typename T>
    static T sqr(const T &a) {
      return a * a;
    }

    static auto Dot(const Vector3D &a, const Vector3D &b) {
      return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static auto Dot2D(const Vector3D &a, const Vector3D &b) {
      return a.x * b.x + a.y * b.y;
    }

    static double DistanceSquared(const Vector3D &a, const Vector3D &b) {
      return sqr(b.x - a.x) + sqr(b.y - a.y) + sqr(b.z - a.z);
    }

    static double DistanceSquared2D(const Vector3D &a, const Vector3D &b) {
      return sqr<double>(b.x - a.x) + sqr<double>(b.y - a.y);
    }

    static double Distance(const Vector3D &a, const Vector3D &b) {
      return std::sqrt(DistanceSquared(a, b));
    }

    static double Distance2D(const Vector3D &a, const Vector3D &b) {
      return std::sqrt(DistanceSquared2D(a, b));
    }

    /// Returns a pair containing:
    /// - @a first:  distance from v to p' where p' = p projected on segment (w - v)
    /// - @a second: euclidean distance from p to p'
    ///   @param p point to calculate distance
    ///   @param v first point of the segment
    ///   @param w second point of the segment
    static std::pair<double, double> DistSegmentPoint(
        const Vector3D &p,
        const Vector3D &v,
        const Vector3D &w) {
      const double l2 = DistanceSquared2D(v, w);
      const double l = std::sqrt(l2);
      if (l2 == 0.0) {
        return std::make_pair(0.0, Distance2D(v, p));
      }
      const double dot_p_w = Dot2D(p - v, w - v);
      const double t = clamp01(dot_p_w / l2);
      const Vector3D projection = v + t * (w - v);
      return std::make_pair(t * l, Distance2D(projection, p));
    }

    static Vector3D RotatePointOnOrigin2D(Vector3D p, double angle) {
      double s = std::sin(angle);
      double c = std::cos(angle);
      return Vector3D(p.x * c - p.y * s, p.x * s + p.y * c, 0.0);
    }

    /// Returns a pair containing:
    /// - @a first:  distance across the arc from start_pos to p' where p' = p projected on Arc
    /// - @a second: euclidean distance from p to p'
    static std::pair<double, double> DistArcPoint(
        const Vector3D &p,
        const Vector3D &start_pos,
        const double length,
        const double heading, // [radians]
        const double curvature) {
      // transport point relative to the arc starting poistion and rotation
      const Vector3D rotated_p(RotatePointOnOrigin2D(p - start_pos, -heading));

      // find intersection position using the unit vector from the center
      // of the circle to the point and multiplying by the radius
      const double radius = 1.0 / curvature;
      const Vector3D circ_center(0, radius, 0);
      const Vector3D intersection = ((rotated_p - circ_center).MakeUnitVector() * radius) + circ_center;

      // check if the point is in the center of the circle, so we know p
      // is in the same distance of every possible point in the arc
      if (rotated_p == circ_center) {
        return std::make_pair(0.0, radius);
      }

      // use the arc length to calculate the angle in the last point of it
      // circumference of a circle = 2 * PI * r
      const double circumf = 2 * pi() * radius;
      const double last_point_angle = (length / circumf) * pi_double();

      // move the point relative to the center of the circle and find
      // the angle between the point and the center of coords in rad
      const double angle = std::atan2(intersection.y - radius, intersection.x) + pi_half();

      // see if the angle is between 0 and last_point_angle
      DEBUG_ASSERT(angle >= 0);
      if (angle <= last_point_angle) {
        return std::make_pair(
            angle * radius,
            Distance2D(intersection, rotated_p));
      }

      // find the nearest point, start or end to intersection
      const double start_dist = Distance2D(Vector3D(), rotated_p);
      Vector3D end_pos(
          (radius * std::cos(last_point_angle - pi_half())) + circ_center.x,
          (radius * std::sin(last_point_angle - pi_half())) + circ_center.y, 0.0);
      const double end_dist = Distance2D(end_pos, rotated_p);
      return (start_dist < end_dist) ?
          std::make_pair(0.0, start_dist) :
          std::make_pair(length, end_dist);
    }

    static bool PointInRectangle(
        const Vector3D &pos,
        const Vector3D &extent,
        double angle, // [radians]
        const Vector3D &p) {
      // Move p relative to pos's position and angle
      Vector3D transf_p = RotatePointOnOrigin2D(p - pos, -angle);
      return transf_p.x <=  extent.x && transf_p.y <=  extent.y &&
             transf_p.x >= -extent.x && transf_p.y >= -extent.y;
    }
  };

} // namespace geom
} // namespace carla
