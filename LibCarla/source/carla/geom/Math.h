// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
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
  class Quaternion;

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
    static constexpr T Pi_2() {
      static_assert(std::is_floating_point<T>::value, "type must be floating point");
      return static_cast<T>(static_cast<T>(0.5) * Pi<T>());
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

    static auto Cross(const Vector3D &a, const Vector3D &b) {
      return Vector3D(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    }

    static auto Dot(const Vector3D &a, const Vector3D &b) {
      return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static auto Length(const Vector3D &a) {
      return std::sqrt(Math::Dot(a, a));
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

    static float LinearLerp(float a, float b, float f) {
      return a * (1.0f - f) + (b * f);
    }

    /// Returns the angle between 2 vectors in radians in  [-PI; PI]
    static float GetVectorAngle(const Vector3D &a, const Vector3D &b);

    /// Returns the abs angle between 2 vectors in radians in [0; PI]
    static float GetVectorAngleAbs(const Vector3D &a, const Vector3D &b);

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

    /// Compute the unit vector pointing towards the X-axis of @a quaternion.
    static Vector3D GetForwardVector(const Quaternion &quaternion);

    /// Compute the unit vector pointing towards the Y-axis of @a quaternion.
    static Vector3D GetRightVector(const Quaternion &quaternion);

    /// Compute the unit vector pointing towards the Y-axis of @a quaternion.
    static Vector3D GetUpVector(const Quaternion &quaternion);

    // Helper function to generate a vector of consecutive integers from a to b
    static std::vector<int> GenerateRange(int a, int b);

    /** Returns the cosine of the angle between the two unit vectors
     * 
     * This is based on the DotProduct() of the vectors
     * (a * b)/(||a|| * ||b||)= cos(phi)
     * 
     * Be aware: use this optimized function if you are sure that vectors a_unit and b_unit are actually a unit vectors!
     *
     * \param a_unit must be a unit vector
     * \param b_unit must be a unit vector
     * 
     * \returns The cosine of the angle between the two given unit vectors
     */
    static float CosineVectorAngleFromUnitVectors(Vector3D const &a_unit, Vector3D const &b_unit)
    {
        return Dot(a_unit, b_unit);
    }

    /** Returns the cosine of the angle between the two vectors
     * 
     * This is based on the DotProduct() of the vectors
     * (a * b)/(||a|| * ||b||)= cos(phi)
     * 
     * \param a just a vector
     * \param b another vector
     * 
     * \returns The cosine of the angle between the two given vectors
     */
    static float CosineVectorAngle(Vector3D const &a, Vector3D const &b)
    {
      return CosineVectorAngleFromUnitVectors(a.MakeUnitVector(), b.MakeUnitVector());
    }

    /** Returns the absolute of the sine of the angle between the two vectors
     * 
     * This is based on the Cross() of the vectors
     *   a x b = ||a|| * ||b|| * sin(phi) * n
     * with n being the unit vector of the vector with right angles to both a and b
     * 
     * \param a just a vector
     * \param b another vector
     * 
     * \returns The absolute of the sine of the angle between the two given vectors
     */
    static float SineVectorAngleAbs(Vector3D const &a, Vector3D const &b) 
    {
      return Math::Length(Math::Cross(a.MakeUnitVector(), b.MakeUnitVector()));
    }

    /** Returns the sine of the angle between the two unit vectors whereas the sign is dertermined in respect to the up-vector
     * 
     * This is based on the Cross() of the vectors
     *   a x b = ||a|| * ||b|| * sin(phi) * n
     * with n being the unit vector of the vector with right angles to both a and b
     * 
     * Be aware: use this optimized function if you are sure that vectors a_unit and b_unit are actually a unit vectors!
     *
     * \param a_unit must be a unit vector
     * \param b_unit must be a unit vector
     * 
     * \returns The absolute of the sine of the angle between the two given vectors
     */
    static float SineVectorAngleFromUnitVectors(Vector3D const &a_unit, Vector3D const &b_unit);

    /** Returns the sine of the angle between the two vectors whereas the sign is dertermined in respect to the up-vector
     * 
     * This is based on the Cross() of the vectors
     *   a x b = ||a|| * ||b|| * sin(phi) * n
     * with n being the unit vector of the vector with right angles to both a and b
     * 
     * \param a just a vector
     * \param b another vector
     * 
     * \returns The absolute of the sine of the angle between the two given vectors
     */
    static float SineVectorAngle(Vector3D const &a, Vector3D const &b)
    {
      return SineVectorAngleFromUnitVectors(a.MakeUnitVector(), b.MakeUnitVector());
    }

    /** Returns the signbit of the cosine of the angle between vector \c a and unit vector \c b_unit
     * 
     * This is based on the DotProdcut() of the vectors
     * a⋅(b/||b||)=||a|| cos(phi)
     * 
     * \param a just a vector
     * \param b_unit must be a unit vector
     * 
     * \returns the signbit of the cosine of the angle between a vector and a unit vectors (Be aware: +0.f returns \c false, -0.f returns \c true)
     * \retval \c false The vectors are pointing within the same half plane (||phi|| <= M_PI_2) i.e. pointing into "same direction"
     * \retval \c true The vectors are pointing  the same half plane (||phi|| >= M_PI_2) i.e. pointing into "opposite direction"
     * 
     * Be aware: use this optimized function if you are sure that vector b_unit is actually a unit vector!
     */
    static bool SignBitCosineAngleFromUnitVector(Vector3D const &a, Vector3D const &b_unit)
    {
      return std::signbit(Dot(a, b_unit));
    }

    /** Returns the signbit of the cosine of the angle between two vectors
     * 
     * This is based on the DotProdcut() of the vectors
     * a⋅(b/||b||)=||a|| cos(phi)
     * 
     * \param a just a vector
     * \param b another vector
     * 
     * \returns the signbit of the cosine of the angle between the two vectors (Be aware: +0.f returns \c false, -0.f returns \c true)
     * \retval \c false The vectors are pointing within the same half plane (||phi|| <= M_PI_2) i.e. pointing into "same direction"
     * \retval \c true The vectors are pointing  the same half plane (||phi|| >= M_PI_2) i.e. pointing into "opposite direction"
     */
    static bool SignBitCosineAngle(Vector3D const &a, Vector3D const &b)
    {
      return SignBitCosineAngleFromUnitVector(a, b.MakeUnitVector());
    }

  };
} // namespace geom
} // namespace carla
