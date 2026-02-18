// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/geom/Math.h"

#include "carla/geom/Rotation.h"
#include "carla/geom/Quaternion.h"

namespace carla {
namespace geom {

  float Math::SineVectorAngleFromUnitVectors(Vector3D const &a_unit, Vector3D const &b_unit)
  {
    auto const cross = Math::Cross(a_unit, b_unit);
    Vector3D const to_up(0.f, 0.f, 1.f);
    auto const sine_angle_abs = Math::Length(cross);
    if ( std::signbit(Math::Dot(cross, to_up) ) )
    {
      return -sine_angle_abs;
    }
    else
    {
      return sine_angle_abs;
    }
  }

  float Math::GetVectorAngleAbs(const Vector3D &a, const Vector3D &b) {
    float cosine_vector_angle = Math::CosineVectorAngle(a, b);
    return std::acos(cosine_vector_angle);
  }

  float Math::GetVectorAngle(const Vector3D &a, const Vector3D &b) {
    auto const a_unit = a.MakeUnitVector();
    auto const b_unit = b.MakeUnitVector();
    auto const cosine_vector_angle = Math::CosineVectorAngleFromUnitVectors(a_unit, b_unit);
    auto const sine_vector_angle = Math::SineVectorAngleFromUnitVectors(a_unit, b_unit);
    auto const angle = std::atan2(sine_vector_angle, cosine_vector_angle);
    return angle;
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

  Vector3D GetForwardVector(const Rotation &rotation) { return rotation.GetForwardVector(); }

  Vector3D GetRightVector(const Rotation &rotation) { return rotation.GetRightVector(); }

  Vector3D GetUpVector(const Rotation &rotation) { return rotation.GetUpVector(); }

  Vector3D GetForwardVector(const Quaternion &quaternion) { return quaternion.GetForwardVector(); }

  Vector3D GetRightVector(const Quaternion &quaternion) { return quaternion.GetRightVector(); }

  Vector3D GetUpVector(const Quaternion &quaternion) { return quaternion.GetUpVector(); }

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
