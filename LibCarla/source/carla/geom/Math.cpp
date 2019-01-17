// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/geom/Math.h"

#include "carla/geom/Rotation.h"

namespace carla {
namespace geom {

  /// Returns a pair containing:
  /// - @b first:  distance from v to p' where p' = p projected on segment (w -
  /// v)
  /// - @b second: Euclidean distance from p to p'
  ///   @param p point to calculate distance
  ///   @param v first point of the segment
  ///   @param w second point of the segment
  std::pair<double, double> Math::DistSegmentPoint(
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

  Vector3D Math::RotatePointOnOrigin2D(Vector3D p, double angle) {
    double s = std::sin(angle);
    double c = std::cos(angle);
    return Vector3D(p.x * c - p.y * s, p.x * s + p.y * c, 0.0);
  }

  /// Returns a pair containing:
  /// - @b first:  distance across the arc from start_pos to p' where p' = p
  /// projected on Arc
  /// - @b second: Euclidean distance from p to p'
  std::pair<double, double> Math::DistArcPoint(
      Vector3D p,
      Vector3D start_pos,
      double length,
      double heading,   // [radians]
      double curvature) {

    /// @todo: Because Unreal's coordinates, hacky way to correct
    /// the -y, this must be changed in the future
    p.y = -p.y;
    start_pos.y = -start_pos.y;
    heading = -heading;

    // since this algorithm is working for positive curvatures,
    // and we are only calculating distances, we can invert the y
    // axis (along with the curvature and the heading), so if the
    // curvature is negative, so the algorithm will work as expected
    if (curvature < 0.0) {
      p.y = -p.y;
      start_pos.y = -start_pos.y;
      heading = -heading;
      curvature = -curvature;
    }

    // transport point relative to the arc starting poistion and rotation
    const Vector3D rotated_p(RotatePointOnOrigin2D(p - start_pos, -heading));

    const double radius = 1.0 / curvature;
    const Vector3D circ_center(0, radius, 0);

    // check if the point is in the center of the circle, so we know p
    // is in the same distance of every possible point in the arc
    if (rotated_p == circ_center) {
      return std::make_pair(0.0, radius);
    }

    // find intersection position using the unit vector from the center
    // of the circle to the point and multiplying by the radius
    const Vector3D intersection = ((rotated_p - circ_center).MakeUnitVector() * radius) + circ_center;

    // use the arc length to calculate the angle in the last point of it
    // circumference of a circle = 2 * PI * r
    // last_point_angle = (length / circumference) * 2 * PI
    // so last_point_angle = length / radius
    const double last_point_angle = length / radius;

    // move the point relative to the center of the circle and find
    // the angle between the point and the center of coords in rad
    double angle = std::atan2(intersection.y - radius, intersection.x) + pi_half();

    if (angle < 0.0) {
      angle += pi_double();
    }

    // see if the angle is between 0 and last_point_angle
    DEBUG_ASSERT(angle >= 0.0);
    if (angle <= last_point_angle) {
      return std::make_pair(
          angle * radius,
          Distance2D(intersection, rotated_p));
    }

    // find the nearest point, start or end to intersection
    const double start_dist = Distance2D(Vector3D(), rotated_p);

    const Vector3D end_pos(
        radius * std::cos(last_point_angle - pi_half()),
        radius *std::sin(last_point_angle - pi_half()) + circ_center.y,
        0.0);
    const double end_dist = Distance2D(end_pos, rotated_p);
    return (start_dist < end_dist) ?
           std::make_pair(0.0, start_dist) :
           std::make_pair(length, end_dist);
  }

  bool Math::PointInRectangle(
      const Vector3D &pos,
      const Vector3D &extent,
      double angle,   // [radians]
      const Vector3D &p) {
    // Move p relative to pos's position and angle
    Vector3D transf_p = RotatePointOnOrigin2D(p - pos, -angle);
    return transf_p.x <=  extent.x && transf_p.y <=  extent.y &&
           transf_p.x >= -extent.x && transf_p.y >= -extent.y;
  }

  Vector3D Math::GetForwardVector(const Rotation &rotation) {
    const float cp = std::cos(to_radians(rotation.pitch));
    const float sp = std::sin(to_radians(rotation.pitch));
    const float cy = std::cos(to_radians(rotation.yaw));
    const float sy = std::sin(to_radians(rotation.yaw));
    return {cy * cp, sy * cp, sp};
  }

} // namespace geom
} // namespace carla
