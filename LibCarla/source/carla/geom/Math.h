// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"

#include <cmath>

namespace carla {
namespace geom {

  class Math {
  public:

    static constexpr auto pi() {
      return 3.14159265358979323846264338327950288;
    }

    static constexpr auto pi_half() {
      return 0.5 * pi();
    }

    static double clamp01(double a) {
      return std::min(std::max(a, 0.0), 1.0);
    }

    template<typename T>
    static T sqr(const T &a) {
      return a * a;
    }

    static auto Dot(const Location &a, const Location &b) {
      return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static auto Dot2D(const Location &a, const Location &b) {
      return a.x * b.x + a.y * b.y;
    }

    static double DistanceSquared(const Location &a, const Location &b) {
      return sqr(b.x - a.x) + sqr(b.y - a.y) + sqr(b.z - a.z);
    }

    static double DistanceSquared2D(const Location &a, const Location &b) {
      return sqr<double>(b.x - a.x) + sqr<double>(b.y - a.y);
    }

    static double Distance(const Location &a, const Location &b) {
      return std::sqrt(DistanceSquared(a, b));
    }

    static double Distance2D(const Location &a, const Location &b) {
      return std::sqrt(DistanceSquared2D(a, b));
    }

    static double DistanceSegmentPoint(
        const Location &p,
        const Location &v,
        const Location &w) {
      const double l2 = DistanceSquared2D(v, w);
      if (l2 == 0.0) {
        return Distance2D(p, v);
      }
      const double t = clamp01(Dot2D(p - v, w - v) / l2);
      const Location projection = v + t * (w - v);
      return Distance2D(p, projection);
    }
  };

} // namespace geom
} // namespace carla
