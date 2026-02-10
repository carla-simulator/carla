// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <array>
#include <ostream>

#include "carla/MsgPack.h"
#include "carla/geom/Math.h"
#include "carla/geom/Location.h"
#include "carla/geom/Vector3D.h"
#include "carla/geom/Rotation.h"

namespace carla {
namespace geom {

  class Quaternion;
  class Transform;

  Quaternion operator*(const Quaternion& q1, const Quaternion& q2);
  Quaternion operator*(const Quaternion& q, const Vector3D& w);
  Quaternion operator*(const Vector3D& w, const Quaternion& q);
  Quaternion operator*(const Quaternion& q1, const float& s);

  /**
   * Stores the orientation of an entity as quaternion; 
   * The quaternion is existing in Unreal coordinate system.
   * This is considered by all defined operations on the quaternion (input/output vectors and angles are automatically converted where required)
  */
  class Quaternion {
  public:

    // =========================================================================
    // -- Public data members --------------------------------------------------
    // =========================================================================

    float x = 0.0f;

    float y = 0.0f;

    float z = 0.0f;

    float w = 1.0f;

    // =========================================================================
    // -- Constructors and "Creators" ---------------------------------------------------------
    // =========================================================================

    Quaternion(float x_ = 0.f, float y_= 0.f, float z_= 0.f, float w_= 1.f)
      : x(x_),
        y(y_),
        z(z_),
        w(w_) {}

    explicit Quaternion(Rotation const &rotator) {
      // intermediate values in double to improve precision
      // calculation see https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
      // geom::Quaternion is in unreal left handed system, rotating around all axis left handed
      // geom::Rotation is in unreal left handed system, treating yaw as left-handed rotation
      // but pitch and roll as right-handed ones! Therefore, we have to negate pitch and roll here
      double const roll_2 = Math::ToRadians(-rotator.roll) * 0.5;
      double const pitch_2 = Math::ToRadians(-rotator.pitch) * 0.5;
      double const yaw_2 = Math::ToRadians(rotator.yaw) * 0.5;
      double cr = std::cos(roll_2);
      double sr = std::sin(roll_2);
      double cp = std::cos(pitch_2);
      double sp = std::sin(pitch_2);
      double cy = std::cos(yaw_2);
      double sy = std::sin(yaw_2);

      x = float(sr * cp * cy - cr * sp * sy);
      y = float(cr * sp * cy + sr * cp * sy);
      z = float(cr * cp * sy - sr * sp * cy);
      w = float(cr * cp * cy + sr * sp * sy);
    }

    static Quaternion CreateFromYawDegree(float const &yaw_degree) {
      // intermediate values in double to improve precision
      // calculation see https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
      double const yaw_2 = Math::ToRadians(yaw_degree) * 0.5;
      double cy = std::cos(yaw_2);
      double sy = std::sin(yaw_2);
      Quaternion quat;
      // geom::Quaternion is in unreal left handed system, rotating around all axis left handed
      // geom::Rotation is in unreal left handed system, treating yaw as left-handed rotation
      // Therefore, no negation required for yaw component
      quat.x = 0.f;
      quat.y = 0.f;
      quat.z = float(sy);
      quat.w = float(cy);
      return quat;
    }

    static Quaternion Identity() { return Quaternion(); }

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    /// Compute the unit vector pointing towards the X-axis of the rotation described by this quaternion.
    Vector3D GetForwardVector() const {
      Vector3D const forward_vector(1.0, 0.0, 0.0);
      return RotatedVector(forward_vector);
    }

    /// Compute the unit vector pointing towards the Y-axis of of the rotation described by this quaternion.
    Vector3D GetRightVector() const {
      Vector3D const right_vector(0.0, 1.0, 0.0);
      return RotatedVector(right_vector);
    }

    /// Compute the unit vector pointing towards the Z-axis of of the rotation described by this quaternion.
    Vector3D GetUpVector() const {
      Vector3D const up_vector(0.0, 0.0, 1.0);
      return RotatedVector(up_vector);
    }

    Quaternion Inverse() const {
      return Quaternion(-x, -y, -z, w);
    }

    float Length() const {
      return std::sqrt(SquaredLength());
    }

    float SquaredLength() const {
      return x * x + y * y + z * z + w * w;
    }

    Quaternion UnitQuaternion() const {
      auto const d = Length();
      Quaternion unit_quat = *this * d;
      return unit_quat;
    }

    /* Return this quaternion rotated by the give quaternion \c q */
    Quaternion RotatedQuaternion(Quaternion const &q) const {
      return q * *this * q.Inverse();
    }

    /** shortened version of Rotator() providing the yaw component in rad
     */
    float YawRad() const {
      auto const matrix = RotationMatrix();
      float yaw = 0.f;
      if (std::abs(matrix[6]) < 1.f) {
        auto const pitch = -std::asin(matrix[6]);
        auto const cp = std::cos(pitch);
        yaw = std::atan2(matrix[3]/cp, matrix[0]/cp);
      }
      // geom::Quaternion is in unreal left handed system, rotating around all axis left handed
      // geom::Rotation is in unreal left handed system, treating yaw as left-handed rotation
      // Therefore, no negation required for yaw component
      return yaw;
    }

    /** shortened version of Rotator() providing the yaw component in degree
     */
    float YawDegree() const {
      return Math::ToDegrees(YawRad());
    }

    carla::geom::Rotation Rotator() const {
      // calculation see https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
      auto const matrix = RotationMatrix();
      float yaw = 0.f;
      float pitch = 0.f;
      float roll = 0.f;
      if (std::abs(matrix[6]) < 1.f) {
        pitch = -std::asin(matrix[6]);
        auto const cp = std::cos(pitch);
        yaw = std::atan2(matrix[3]/cp, matrix[0]/cp);
        roll = std::atan2(matrix[7]/cp, matrix[8]/cp);
      }
      else {
        // singularity
        if ( std::signbit(matrix[6]) ) {
          // gimbal lock up
          pitch = Math::Pi_2<float>();
        } 
        else {
          // gimbal lock down
          pitch = -Math::Pi_2<float>();
        }
        yaw = 0.f;
        roll = std::atan2(matrix[7], matrix[8]);
      }
      // geom::Quaternion is in unreal left handed system, rotating around all axis left handed
      // geom::Rotation is in unreal left handed system, treating yaw as left-handed rotation
      // but pitch and roll as right-handed ones! Therefore, we have to negate pitch and roll here
      carla::geom::Rotation rotator;
      rotator.roll = Math::ToDegrees(-roll);
      rotator.pitch = Math::ToDegrees(-pitch);
      rotator.yaw = Math::ToDegrees(yaw);
      return rotator;
    }

    template <class VECTOR_TYPE>
    Vector3D RotatedVector(VECTOR_TYPE const &in_point) const {
       return RotatedPoint(in_point);
    }

    template <class VECTOR_TYPE>
    Vector3D InverseRotatedVector(VECTOR_TYPE const &in_point) const {
      return InverseRotatedPoint(in_point);
    }

   // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const Quaternion &rhs) const  {
      return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
    }

    bool operator!=(const Quaternion &rhs) const  {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- Conversions to UE4 types ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Quaternion(const FQuat &quat)
      : x(quat.X)
      , y(quat.Y)
      , z(quat.Z)
      , w(quat.W) {}

    operator FQuat() const {
      FQuat quat;
      quat.X = x;
      quat.Y = y;
      quat.Z = z;
      quat.W = w;
      return quat;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4


    // =========================================================================
    /// @todo The following is copy-pasted from MSGPACK_DEFINE_ARRAY.
    /// This is a workaround for an issue in msgpack library. The
    /// MSGPACK_DEFINE_ARRAY macro is shadowing our `z` variable.
    /// https://github.com/msgpack/msgpack-c/issues/709
    // =========================================================================
    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        clmdep_msgpack::type::make_define_array(x, y, z, w).msgpack_pack(pk);
    }
    void msgpack_unpack(clmdep_msgpack::object const& o)
    {
        clmdep_msgpack::type::make_define_array(x, y, z, w).msgpack_unpack(o);
    }
    template <typename MSGPACK_OBJECT>
    void msgpack_object(MSGPACK_OBJECT* o, clmdep_msgpack::zone& sneaky_variable_that_shadows_z) const
    {
        clmdep_msgpack::type::make_define_array(x, y, z, w).msgpack_object(o, sneaky_variable_that_shadows_z);
    }
    // =========================================================================

  public:
    std::array<float, 9> RotationMatrix() const {
      // calculation see https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
      // in the matrix each component is multplied with another component; 
      // instead of dividing each component by the norm, and the multiply (and square the norm again), we can use the squard_norm immediately
      // the factor of 2 can also be moved into the single components
      auto const norm_squared = SquaredLength();
      auto const factor = 2.f / norm_squared;
      auto const x2 = x * factor;
      auto const y2 = y * factor;
      auto const z2 = z * factor;
      auto const wx2 = w * x2;
      auto const wy2 = w * y2;
      auto const wz2 = w * z2;
      auto const xx2 = x * x2;
      auto const xy2 = x * y2;
      auto const xz2 = x * z2;
      auto const yy2 = y * y2;
      auto const yz2 = y * z2;
      auto const zz2 = z * z2;
      std::array<float, 9> matrix = {
          float(1. - (yy2 + zz2)),
          float(xy2 - wz2),
          float(wy2 + xz2),
          float(xy2 + wz2),
          float(1. - (xx2 + zz2)),
          float(yz2 - wx2),
          float(xz2 - wy2),
          float(wx2 + yz2),
          float(1. - (xx2 + yy2))};
      return matrix;
    }

    /// Computes the 3x3 rotation-matrix of the inverse quaternion
    std::array<float, 9> InverseRotationMatrix() const {
      return Inverse().RotationMatrix();
    }

    Vector3D RotatedPoint(Vector3D const &in_point) const {
      Quaternion quat = *this * in_point * Inverse();
      Vector3D out_point;
      out_point.x = quat.x;
      out_point.y = quat.y;
      out_point.z = quat.z;
      return out_point;
    }

    Vector3D InverseRotatedPoint(Vector3D const &in_point) const {
      Quaternion quat = Inverse() * in_point * *this;
      Vector3D out_point;
      out_point.x = quat.x;
      out_point.y = quat.y;
      out_point.z = quat.z;
      return out_point;
    }
  };


inline Quaternion operator*(const Quaternion& q1, const Quaternion& q2) {
	Quaternion quat;
  quat.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
  quat.y = q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z;
  quat.z = q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x;
  quat.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
  return quat;
}

inline Quaternion operator*(const Quaternion& q, const Vector3D& v)
{
	Quaternion quat;
  quat.x = q.w * v.x + q.y * v.z - q.z * v.y;
  quat.y = q.w * v.y + q.z * v.x - q.x * v.z;
	quat.z = q.w * v.z + q.x * v.y - q.y * v.x;
	quat.w = -q.x * v.x - q.y * v.y - q.z * v.z; 
  return quat;
}

inline Quaternion operator*(const Vector3D& v, const Quaternion& q)
{
	Quaternion quat;
  quat.x = v.x * q.w + v.y * q.z - v.z * q.y;
	quat.y = v.y * q.w + v.z * q.x - v.x * q.z;
	quat.z = v.z * q.w + v.x * q.y - v.y * q.x;
	quat.w =-v.x * q.x - v.y * q.y - v.z * q.z; 
  return quat;
}

inline Quaternion operator*(const Quaternion& q1, const float& s) {
	Quaternion quat;
  quat.w = q1.w * s;
  quat.x = q1.x * s;
  quat.y = q1.y * s;
  quat.z = q1.z * s;
  return quat;
}


inline std::ostream &operator<<(std::ostream &out, const Quaternion &quaternion) {
    out << "Quaternion"
        << "(x=" << std::to_string(quaternion.x)
        << ", y=" << std::to_string(quaternion.y)
        << ", z=" << std::to_string(quaternion.z)
        << ", w=" << std::to_string(quaternion.w) << ')';
    return out;
  }

} // namespace geom
} // namespace carla
