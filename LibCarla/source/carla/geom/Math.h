// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/geom/Vector3D.h"

#include <cmath>
#include <type_traits>
#include <utility>

namespace carla {
namespace geom {

  class Rotation;

  class Math {
  public:

    template <typename T>
    static constexpr T Pi() {
      static_assert(std::is_floating_point<T>::value, "type must be floating point");
      return static_cast<T>(3.14159265358979323846264338327950288);
    }

    template <typename T>
    static constexpr T Pi2() {
      static_assert(std::is_floating_point<T>::value, "type must be floating point");
      return static_cast<T>(static_cast<T>(2) * Pi<T>());
    }

    template <typename T>
    static constexpr T ToDegrees(T rad) {
      static_assert(std::is_floating_point<T>::value, "type must be floating point");
      return rad * (T(180.0) / Pi<T>());
    }

    template <typename T>
    static constexpr T ToRadians(T deg) {
      static_assert(std::is_floating_point<T>::value, "type must be floating point");
      return deg * (Pi<T>() / T(180.0));
    }

    template <typename T>
    static T Clamp(T a, T min = T(0), T max = T(1)) {
      return std::min(std::max(a, min), max);
    }

    template <typename T>
    static T Square(const T &a) {
      return a * a;
    }

    static auto Dot(const Vector3D &a, const Vector3D &b) {
      return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static auto Dot2D(const Vector3D &a, const Vector3D &b) {
      return a.x * b.x + a.y * b.y;
    }

    static auto DistanceSquared(const Vector3D &a, const Vector3D &b) {
      return Square(b.x - a.x) + Square(b.y - a.y) + Square(b.z - a.z);
    }

    static auto DistanceSquared2D(const Vector3D &a, const Vector3D &b) {
      return Square(b.x - a.x) + Square(b.y - a.y);
    }

    static auto Distance(const Vector3D &a, const Vector3D &b) {
      return std::sqrt(DistanceSquared(a, b));
    }

    static auto Distance2D(const Vector3D &a, const Vector3D &b) {
      return std::sqrt(DistanceSquared2D(a, b));
    }

    /// Returns the angle between 2 vectors in radians
    static double GetVectorAngle(const Vector3D &a, const Vector3D &b);

    /// Returns a pair containing:
    /// - @b first:  distance from v to p' where p' = p projected on segment
    ///   (w - v)
    /// - @b second: Euclidean distance from p to p'
    ///   @param p point to calculate distance
    ///   @param v first point of the segment
    ///   @param w second point of the segment
    static std::pair<float, float> DistanceSegmentToPoint(
        const Vector3D &p,
        const Vector3D &v,
        const Vector3D &w);

    /// Returns a pair containing:
    /// - @b first:  distance across the arc from start_pos to p' where p' = p
    /// projected on Arc
    /// - @b second: Euclidean distance from p to p'
    static std::pair<float, float> DistanceArcToPoint(
        Vector3D p,
        Vector3D start_pos,
        float length,
        float heading,   // [radians]
        float curvature);

    static Vector3D RotatePointOnOrigin2D(Vector3D p, float angle);

    /// Compute the unit vector pointing towards the X-axis of @a rotation.
    static Vector3D GetForwardVector(const Rotation &rotation);

    /// Compute the unit vector pointing towards the Y-axis of @a rotation.
    static Vector3D GetRightVector(const Rotation &rotation);

    /// Compute the unit vector pointing towards the Y-axis of @a rotation.
    static Vector3D GetUpVector(const Rotation &rotation);

    // Helper function to generate a vector of consecutive integers from a to b
    static std::vector<int> GenerateRange(int a, int b);

  };
} // namespace geom
} // namespace carla
