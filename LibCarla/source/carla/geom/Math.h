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

  class Rotation;

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

    static constexpr auto to_degrees(double rad) {
      return rad * (180.0 / pi());
    }

    static constexpr auto to_radians(double deg) {
      return deg * (pi() / 180.0);
    }

    template <typename T>
    static T clamp(
        const T &a,
        const T &min,
        const T &max) {
      return std::min(std::max(a, min), max);
    }

    template <typename T>
    static T clamp01(T a) {
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

    static auto DistanceSquared(const Vector3D &a, const Vector3D &b) {
      return sqr(b.x - a.x) + sqr(b.y - a.y) + sqr(b.z - a.z);
    }

    static auto DistanceSquared2D(const Vector3D &a, const Vector3D &b) {
      return sqr(b.x - a.x) + sqr(b.y - a.y);
    }

    static auto Distance(const Vector3D &a, const Vector3D &b) {
      return std::sqrt(DistanceSquared(a, b));
    }

    static auto Distance2D(const Vector3D &a, const Vector3D &b) {
      return std::sqrt(DistanceSquared2D(a, b));
    }

    static std::pair<double, double> DistSegmentPoint(
        const Vector3D &,
        const Vector3D &,
        const Vector3D &);

    static Vector3D RotatePointOnOrigin2D(Vector3D p, double angle);

    static std::pair<double, double> DistArcPoint(
        Vector3D,
        Vector3D,
        double,
        double, // [radians]
        double);

    static bool PointInRectangle(
        const Vector3D &,
        const Vector3D &,
        double, // [radians]
        const Vector3D &);

    /// Compute the unit vector pointing towards the X-axis of @a rotation.
    static Vector3D GetForwardVector(const Rotation &rotation);
  };

} // namespace geom
} // namespace carla
