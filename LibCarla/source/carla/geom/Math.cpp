// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/geom/Math.h"

#include "carla/geom/Rotation.h"

namespace carla {
namespace geom {

  double Math::GetVectorAngle(const Vector3D &a, const Vector3D &b) {
    return std::acos(Dot(a, b) / (a.Length() * b.Length()));
  }

  std::pair<float, float> Math::DistanceSegmentToPoint(
      const Vector3D &p,
      const Vector3D &v,
      const Vector3D &w) {
    const float l2 = DistanceSquared2D(v, w);
    const float l = std::sqrt(l2);
    if (l2 == 0.0f) {
      return std::make_pair(0.0f, Distance2D(v, p));
    }
    const float dot_p_w = Dot2D(p - v, w - v);
    const float t = Clamp(dot_p_w / l2);
    const Vector3D projection = v + t * (w - v);
    return std::make_pair(t * l, Distance2D(projection, p));
  }

  std::pair<float, float> Math::DistanceArcToPoint(
      Vector3D p,
      Vector3D start_pos,
      const float length,
      float heading,       // [radians]
      float curvature) {

    /// @todo: Because Unreal's coordinates, hacky way to correct
    /// the -y, this must be changed in the future
    p.y = -p.y;
    start_pos.y = -start_pos.y;
    heading = -heading;
    curvature = -curvature;

    // since this algorithm is working for positive curvatures,
    // and we are only calculating distances, we can invert the y
    // axis (along with the curvature and the heading), so if the
    // curvature is negative, the algorithm will work as expected
    if (curvature < 0.0f) {
      p.y = -p.y;
      start_pos.y = -start_pos.y;
      heading = -heading;
      curvature = -curvature;
    }

    // transport point relative to the arc starting poistion and rotation
    const Vector3D rotated_p(RotatePointOnOrigin2D(p - start_pos, -heading));

    const float radius = 1.0f / curvature;
    const Vector3D circ_center(0.0f, radius, 0.0f);

    // check if the point is in the center of the circle, so we know p
    // is in the same distance of every possible point in the arc
    if (rotated_p == circ_center) {
      return std::make_pair(0.0f, radius);
    }

    // find intersection position using the unit vector from the center
    // of the circle to the point and multiplying by the radius
    const Vector3D intersection = ((rotated_p - circ_center).MakeUnitVector() * radius) + circ_center;

    // use the arc length to calculate the angle in the last point of it
    // circumference of a circle = 2 * PI * r
    // last_point_angle = (length / circumference) * 2 * PI
    // so last_point_angle = length / radius
    const float last_point_angle = length / radius;

    constexpr float pi_half = Pi<float>() / 2.0f;

    // move the point relative to the center of the circle and find
    // the angle between the point and the center of coords in rad
    float angle = std::atan2(intersection.y - radius, intersection.x) + pi_half;

    if (angle < 0.0f) {
      angle += Pi<float>() * 2.0f;
    }

    // see if the angle is between 0 and last_point_angle
    DEBUG_ASSERT(angle >= 0.0f);
    if (angle <= last_point_angle) {
      return std::make_pair(
          angle * radius,
          Distance2D(intersection, rotated_p));
    }

    // find the nearest point, start or end to intersection
    const float start_dist = Distance2D(Vector3D(), rotated_p);

    const Vector3D end_pos(
        radius * std::cos(last_point_angle - pi_half),
        radius * std::sin(last_point_angle - pi_half) + circ_center.y,
        0.0f);
    const float end_dist = Distance2D(end_pos, rotated_p);
    return (start_dist < end_dist) ?
        std::make_pair(0.0f, start_dist) :
        std::make_pair(length, end_dist);
  }

  Vector3D Math::RotatePointOnOrigin2D(Vector3D p, float angle) {
    const float s = std::sin(angle);
    const float c = std::cos(angle);
    return Vector3D(p.x * c - p.y * s, p.x * s + p.y * c, 0.0f);
  }

  Vector3D Math::GetForwardVector(const Rotation &rotation) {
    const float cp = std::cos(ToRadians(rotation.pitch));
    const float sp = std::sin(ToRadians(rotation.pitch));
    const float cy = std::cos(ToRadians(rotation.yaw));
    const float sy = std::sin(ToRadians(rotation.yaw));
    return {cy * cp, sy * cp, sp};
  }

  Vector3D Math::GetRightVector(const Rotation &rotation) {
    const float cy = std::cos(ToRadians(rotation.yaw));
    const float sy = std::sin(ToRadians(rotation.yaw));
    const float cr = std::cos(ToRadians(rotation.roll));
    const float sr = std::sin(ToRadians(rotation.roll));
    const float cp = std::cos(ToRadians(rotation.pitch));
    const float sp = std::sin(ToRadians(rotation.pitch));
    return {
         cy * sp * sr - sy * cr,
         sy * sp * sr + cy * cr,
        -cp * sr};
  }

  Vector3D Math::GetUpVector(const Rotation &rotation) {
    const float cy = std::cos(ToRadians(rotation.yaw));
    const float sy = std::sin(ToRadians(rotation.yaw));
    const float cr = std::cos(ToRadians(rotation.roll));
    const float sr = std::sin(ToRadians(rotation.roll));
    const float cp = std::cos(ToRadians(rotation.pitch));
    const float sp = std::sin(ToRadians(rotation.pitch));
    return {
        -cy * sp * cr - sy * sr,
        -sy * sp * cr + cy * sr,
        cp * cr};
  }

  std::vector<int> Math::GenerateRange(int a, int b) {
    std::vector<int> result;
    if (a < b) {
      for(int i = a; i <= b; ++i) {
        result.push_back(i);
      }
    } else {
      for(int i = a; i >= b; --i) {
        result.push_back(i);
      }
    }
    return result;
  }

} // namespace geom
} // namespace carla
