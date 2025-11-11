// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/geom/BoundingBox.h>
#include <carla/geom/GeoLocation.h>
#include <carla/geom/GeoProjectionsParams.h>
#include <carla/geom/Acceleration.h>
#include <carla/geom/AngularVelocity.h>
#include <carla/geom/Location.h>
#include <carla/geom/Rotation.h>
#include <carla/geom/Quaternion.h>
#include <carla/geom/Transform.h>
#include <carla/geom/Vector2D.h>
#include <carla/geom/Vector3D.h>
#include <carla/geom/Velocity.h>

#include <boost/python/implicit.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <ostream>

namespace carla {
namespace geom {

  template <typename T>
  static void WriteVector2D(std::ostream &out, const char *name, const T &vector2D) {
    out << name
        << "(x=" << std::to_string(vector2D.x)
        << ", y=" << std::to_string(vector2D.y) << ')';
  }

  template <typename T>
  static void WriteVector3D(std::ostream &out, const char *name, const T &vector3D) {
    out << name
        << "(x=" << std::to_string(vector3D.x)
        << ", y=" << std::to_string(vector3D.y)
        << ", z=" << std::to_string(vector3D.z) << ')';
  }

  std::ostream &operator<<(std::ostream &out, const Vector2D &vector2D) {
    WriteVector2D(out, "Vector2D", vector2D);
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Vector3D &vector3D) {
    WriteVector3D(out, "Vector3D", vector3D);
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Location &location) {
    WriteVector3D(out, "Location", location);
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Velocity &vector3D) {
    WriteVector3D(out, "Velocity", vector3D);
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const AngularVelocity &vector3D) {
    WriteVector3D(out, "AngularVelocity", vector3D);
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Acceleration &vector3D) {
    WriteVector3D(out, "Acceleration", vector3D);
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Rotation &rotator) {
    out << "Rotation(pitch=" << std::to_string(rotator.pitch)
        << ", yaw=" << std::to_string(rotator.yaw)
        << ", roll=" << std::to_string(rotator.roll) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Quaternion &quaternion) {
    out << "Quaternion"
        << "(x=" << std::to_string(quaternion.x)
        << ", y=" << std::to_string(quaternion.y)
        << ", z=" << std::to_string(quaternion.z)
        << ", w=" << std::to_string(quaternion.w) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Transform &transform) {
    out << "Transform(" << transform.location << ", " << transform.rotation << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const BoundingBox &box) {
    out << "BoundingBox(" << box.location << ", ";
    WriteVector3D(out, "Extent", box.extent);
    out << ", " << box.rotation;
    out << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const GeoLocation &geo_location) {
    out << "GeoLocation(latitude=" << std::to_string(geo_location.latitude)
        << ", longitude=" << std::to_string(geo_location.longitude)
        << ", altitude=" << std::to_string(geo_location.altitude) << ')';
    return out;
  }

} // namespace geom
} // namespace carla

static void TransformList(const carla::geom::Transform &self, boost::python::list &list) {
  auto length = boost::python::len(list);
   for (auto i = 0u; i < length; ++i) {
    self.TransformPoint(boost::python::extract<carla::geom::Vector3D &>(list[i]));
  }
}

#if ALLOW_UNSAFE_GEOM_MATRIX_ACCESS
static boost::python::list BuildMatrix(const std::array<float, 16> &m) {
  boost::python::list r_out;
  boost::python::list r[4];
  for (uint8_t i = 0; i < 16; ++i) { r[uint8_t(i / 4)].append(m[i]); }
  for (uint8_t i = 0; i < 4; ++i) { r_out.append(r[i]); }
  return r_out;
}

static auto GetTransformMatrix(const carla::geom::Transform &self) {
  return BuildMatrix(self.TransformationMatrix());
}

static auto GetInverseTransformMatrix(const carla::geom::Transform &self) {
  return BuildMatrix(self.InverseTransformationMatrix());
}
#endif

static auto Cross(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::Cross(self, other);
}

static auto Dot(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::Dot(self, other);
}

static auto Distance(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::Distance(self, other);
}

static auto DistanceSquared(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::DistanceSquared(self, other);
}

static auto Dot2D(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::Dot2D(self, other);
}

static auto Distance2D(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::Distance2D(self, other);
}

static auto DistanceSquared2D(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::DistanceSquared2D(self, other);
}

static auto GetVectorAngle(const carla::geom::Vector3D &self, const carla::geom::Vector3D &other) {
  return carla::geom::Math::GetVectorAngle(self, other);
}

void export_geom() {
  using namespace boost::python;
  namespace cg = carla::geom;
  class_<std::vector<cg::Vector2D>>("vector_of_vector2D")
      .def(boost::python::vector_indexing_suite<std::vector<cg::Vector2D>>())
      .def(self_ns::str(self_ns::self))
  ;

  class_<cg::Vector2D>("Vector2D")
    .def(init<float, float>((arg("x")=0.0f, arg("y")=0.0f)))
    .def_readwrite("x", &cg::Vector2D::x)
    .def_readwrite("y", &cg::Vector2D::y)
    .def("squared_length", &cg::Vector2D::SquaredLength)
    .def("length", &cg::Vector2D::Length)
    .def("make_unit_vector", &cg::Vector2D::MakeUnitVector)
    .def("__eq__", &cg::Vector2D::operator==)
    .def("__ne__", &cg::Vector2D::operator!=)
    .def(self += self)
    .def(self + self)
    .def(self -= self)
    .def(self - self)
    .def(self *= double())
    .def(self * double())
    .def(double() * self)
    .def(self /= double())
    .def(self / double())
    .def(double() / self)
    .def(self_ns::str(self_ns::self))
  ;

  implicitly_convertible<cg::Vector3D, cg::Location>();
  implicitly_convertible<cg::Location, cg::Vector3D>();

  class_<cg::Vector3D>("Vector3D")
    .def(init<float, float, float>((arg("x")=0.0f, arg("y")=0.0f, arg("z")=0.0f)))
    .def(init<const cg::Location &>((arg("rhs"))))
    .def_readwrite("x", &cg::Vector3D::x)
    .def_readwrite("y", &cg::Vector3D::y)
    .def_readwrite("z", &cg::Vector3D::z)
    .def("length", &cg::Vector3D::Length)
    .def("squared_length", &cg::Vector3D::SquaredLength)
    .def("make_unit_vector", &cg::Vector3D::MakeUnitVector)
    .def("cross", &Cross, (arg("vector")))
    .def("dot", &Dot, (arg("vector")))
    .def("dot_2d", &Dot2D, (arg("vector")))
    .def("distance", &Distance, (arg("vector")))
    .def("distance_2d", &Distance2D, (arg("vector")))
    .def("distance_squared", &DistanceSquared, (arg("vector")))
    .def("distance_squared_2d", &DistanceSquared2D, (arg("vector")))
    .def("get_vector_angle", &GetVectorAngle, (arg("vector")))
    .def("__eq__", &cg::Vector3D::operator==)
    .def("__ne__", &cg::Vector3D::operator!=)
    .def("__abs__", &cg::Vector3D::Abs)
    .def(self += self)
    .def(self + self)
    .def(self -= self)
    .def(self - self)
    .def(self *= double())
    .def(self * double())
    .def(double() * self)
    .def(self /= double())
    .def(self / double())
    .def(double() / self)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cg::Location, bases<cg::Vector3D>>("Location")
    .def(init<float, float, float>((arg("x")=0.0f, arg("y")=0.0f, arg("z")=0.0f)))
    .def(init<const cg::Vector3D &>((arg("rhs"))))
    .add_property("x", +[](const cg::Location &self) { return self.x; }, +[](cg::Location &self, float x) { self.x = x; })
    .add_property("y", +[](const cg::Location &self) { return self.y; }, +[](cg::Location &self, float y) { self.y = y; })
    .add_property("z", +[](const cg::Location &self) { return self.z; }, +[](cg::Location &self, float z) { self.z = z; })
    .def("distance", &cg::Location::Distance, (arg("location")))
    .def("__eq__", &cg::Location::operator==)
    .def("__ne__", &cg::Location::operator!=)
    .def("__abs__", &cg::Location::Abs)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cg::Acceleration, bases<cg::Vector3D>>("Acceleration")
    .def(init<float, float, float>((arg("x")=0.0f, arg("y")=0.0f, arg("z")=0.0f)))
    .def(init<const cg::Vector3D &>((arg("rhs"))))
    .add_property("x", +[](const cg::Acceleration &self) { return self.x; }, +[](cg::Acceleration &self, float x) { self.x = x; })
    .add_property("y", +[](const cg::Acceleration &self) { return self.y; }, +[](cg::Acceleration &self, float y) { self.y = y; })
    .add_property("z", +[](const cg::Acceleration &self) { return self.z; }, +[](cg::Acceleration &self, float z) { self.z = z; })
    .def("__eq__", &cg::Acceleration::operator==)
    .def("__ne__", &cg::Acceleration::operator!=)
    .def("__abs__", &cg::Acceleration::Abs)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cg::AngularVelocity, bases<cg::Vector3D>>("AngularVelocity")
    .def(init<float, float, float>((arg("x")=0.0f, arg("y")=0.0f, arg("z")=0.0f)))
    .def(init<const cg::Vector3D &>((arg("rhs"))))
    .add_property("x", +[](const cg::AngularVelocity &self) { return self.x; }, +[](cg::AngularVelocity &self, float x) { self.x = x; })
    .add_property("y", +[](const cg::AngularVelocity &self) { return self.y; }, +[](cg::AngularVelocity &self, float y) { self.y = y; })
    .add_property("z", +[](const cg::AngularVelocity &self) { return self.z; }, +[](cg::AngularVelocity &self, float z) { self.z = z; })
    .def("__eq__", &cg::AngularVelocity::operator==)
    .def("__ne__", &cg::AngularVelocity::operator!=)
    .def("__abs__", &cg::AngularVelocity::Abs)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cg::Velocity, bases<cg::Vector3D>>("Velocity")
    .def(init<float, float, float>((arg("x")=0.0f, arg("y")=0.0f, arg("z")=0.0f)))
    .def(init<const cg::Vector3D &>((arg("rhs"))))
    .add_property("x", +[](const cg::Velocity &self) { return self.x; }, +[](cg::Velocity &self, float x) { self.x = x; })
    .add_property("y", +[](const cg::Velocity &self) { return self.y; }, +[](cg::Velocity &self, float y) { self.y = y; })
    .add_property("z", +[](const cg::Velocity &self) { return self.z; }, +[](cg::Velocity &self, float z) { self.z = z; })
    .def("__eq__", &cg::Velocity::operator==)
    .def("__ne__", &cg::Velocity::operator!=)
    .def("__abs__", &cg::Velocity::Abs)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cg::Rotation>("Rotation")
    .def(init<float, float, float>((arg("pitch")=0.0f, arg("yaw")=0.0f, arg("roll")=0.0f)))
    .def_readwrite("pitch", &cg::Rotation::pitch)
    .def_readwrite("yaw", &cg::Rotation::yaw)
    .def_readwrite("roll", &cg::Rotation::roll)
    .def("get_forward_vector", &cg::Rotation::GetForwardVector)
    .def("get_right_vector", &cg::Rotation::GetRightVector)
    .def("get_up_vector", &cg::Rotation::GetUpVector)
    .def("__eq__", &cg::Rotation::operator==)
    .def("__ne__", &cg::Rotation::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cg::Quaternion>("Quaternion")
    .def(init<float, float, float>((arg("x")=0.0f, arg("y")=0.0f, arg("z")=0.0f, arg("w")=1.0f)))
    .def(init<cg::Rotation>((arg("rotator"))))
    .def_readwrite("x", &cg::Quaternion::x)
    .def_readwrite("y", &cg::Quaternion::y)
    .def_readwrite("z", &cg::Quaternion::z)
    .def_readwrite("w", &cg::Quaternion::w)
    .def("create_from_yaw_degree", &cg::Quaternion::CreateFromYawDegree, (arg("yaw")))
    .def("identity", &cg::Quaternion::Identity)
    .def("get_forward_vector", &cg::Quaternion::GetForwardVector)
    .def("get_right_vector", &cg::Quaternion::GetRightVector)
    .def("get_up_vector", &cg::Quaternion::GetUpVector)
    .def("inverse", &cg::Quaternion::Inverse)
    .def("length", &cg::Quaternion::Length)
    .def("squared_length", &cg::Quaternion::SquaredLength)
    .def("unit_quaternion", &cg::Quaternion::UnitQuaternion)
#if ALLOW_UNSAFE_GEOM_MATRIX_ACCESS
    .def("rotation_matrix", &cg::Quaternion::RotationMatrix)
    .def("inverse_rotation_matrix", &cg::Quaternion::InverseRotationMatrix)
#endif
    .def("rotated_quaternion", &cg::Quaternion::RotatedQuaternion, (arg("quaternion")))
    .def("yaw_rad", &cg::Quaternion::YawRad)
    .def("yaw_degree", &cg::Quaternion::YawDegree)
    .def("rotator", &cg::Quaternion::Rotator)
    .def("rotated_vector", &cg::Quaternion::RotatedVector<cg::Vector3D>, (arg("vector")))
    .def("inverse_rotated_vector", &cg::Quaternion::InverseRotatedVector<cg::Vector3D>, (arg("vector")))
    .def("__eq__", &cg::Quaternion::operator==)
    .def("__ne__", &cg::Quaternion::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cg::Transform>("Transform")
    .def(init<cg::Location, cg::Rotation>(
        (arg("location")=cg::Location(), arg("rotation")=cg::Rotation())))
    .def_readwrite("location", &cg::Transform::location)
    .def_readwrite("rotation", &cg::Transform::rotation)
    .def("transform", &TransformList)
    .def("transform", +[](const cg::Transform &self, cg::Vector3D &location) {
      self.TransformPoint(location);
      return location;
    }, arg("in_point"))
    .def("inverse_transform", +[](const cg::Transform &self, cg::Vector3D &location) {
      self.InverseTransformPoint(location);
      return location;
    }, arg("in_point"))
    .def("transform_vector", +[](const cg::Transform &self, cg::Vector3D &vector) {
      self.TransformVector(vector);
      return vector;
    }, arg("in_point"))
    .def("get_forward_vector", &cg::Transform::GetForwardVector)
    .def("get_right_vector", &cg::Transform::GetRightVector)
    .def("get_up_vector", &cg::Transform::GetUpVector)
#if ALLOW_UNSAFE_GEOM_MATRIX_ACCESS
    .def("get_matrix", &GetTransformMatrix)
    .def("get_inverse_matrix", &GetInverseTransformMatrix)
#endif
    .def("__eq__", &cg::Transform::operator==)
    .def("__ne__", &cg::Transform::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<std::vector<cg::Transform>>("vector_of_transform")
      .def(boost::python::vector_indexing_suite<std::vector<cg::Transform>>())
      .def(self_ns::str(self_ns::self))
  ;

  class_<cg::BoundingBox>("BoundingBox")
    .def(init<cg::Location, cg::Vector3D>(
        (arg("location")=cg::Location(), arg("extent")=cg::Vector3D(), arg("rotation")=cg::Rotation())))
    .def_readwrite("location", &cg::BoundingBox::location)
    .def_readwrite("extent", &cg::BoundingBox::extent)
    .def_readwrite("rotation", &cg::BoundingBox::rotation)
    .def_readonly("actor_id", &cg::BoundingBox::actor_id)
    .def("contains", &cg::BoundingBox::Contains, arg("point"), arg("bbox_transform"))
    .def("get_local_vertices", CALL_RETURNING_LIST(cg::BoundingBox, GetLocalVertices))
    .def("get_world_vertices", CALL_RETURNING_LIST_1(cg::BoundingBox, GetWorldVertices, const cg::Transform&), arg("bbox_transform"))
    .def("__eq__", &cg::BoundingBox::operator==)
    .def("__ne__", &cg::BoundingBox::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cg::GeoLocation>("GeoLocation")
    .def(init<double, double, double>((arg("latitude")=0.0, arg("longitude")=0.0, arg("altitude")=0.0)))
    .def_readwrite("latitude", &cg::GeoLocation::latitude)
    .def_readwrite("longitude", &cg::GeoLocation::longitude)
    .def_readwrite("altitude", &cg::GeoLocation::altitude)
    .def("__eq__", &cg::GeoLocation::operator==)
    .def("__ne__", &cg::GeoLocation::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cg::Ellipsoid>("GeoEllipsoid")
    .def(init<double, double>((arg("a")=6378137.0, arg("f_inv")=298.257223563)))
    .def_readwrite("a", &cg::Ellipsoid::a)
    .def_readwrite("f_inv", &cg::Ellipsoid::f_inv)
    .def("__eq__", &cg::Ellipsoid::operator==)
    .def("__ne__", &cg::Ellipsoid::operator!=)
  ;

  class_<cg::TransverseMercatorParams>("GeoProjectionTM")
    .def(init<double, double, double, double, double, cg::Ellipsoid>(
      (arg("lat_0")=0.0, arg("lon_0")=0.0, arg("k")=1.0, arg("x_0")=0.0, arg("y_0")=0.0, arg("ellps")=cg::Ellipsoid())))
    .def_readwrite("lat_0", &cg::TransverseMercatorParams::lat_0)
    .def_readwrite("lon_0", &cg::TransverseMercatorParams::lon_0)
    .def_readwrite("k", &cg::TransverseMercatorParams::k)
    .def_readwrite("x_0", &cg::TransverseMercatorParams::x_0)
    .def_readwrite("y_0", &cg::TransverseMercatorParams::y_0)
    .def_readwrite("ellps", &cg::TransverseMercatorParams::ellps)
    .def("__eq__", &cg::TransverseMercatorParams::operator==)
    .def("__ne__", &cg::TransverseMercatorParams::operator!=)
  ;

  class_<cg::UniversalTransverseMercatorParams>("GeoProjectionUTM")
    .def(init<int, bool, cg::Ellipsoid>((arg("zone")=31, arg("north")=true, arg("ellps")=cg::Ellipsoid())))
    .def_readwrite("zone", &cg::UniversalTransverseMercatorParams::zone)
    .def_readwrite("north", &cg::UniversalTransverseMercatorParams::north)
    .def_readwrite("ellps", &cg::UniversalTransverseMercatorParams::ellps)
    .def("__eq__", &cg::UniversalTransverseMercatorParams::operator==)
    .def("__ne__", &cg::UniversalTransverseMercatorParams::operator!=)
  ;

  class_<cg::WebMercatorParams>("GeoProjectionWebMerc")
    .def(init<cg::Ellipsoid>((arg("ellps")=cg::Ellipsoid())))
    .def_readwrite("ellps", &cg::WebMercatorParams::ellps)
    .def("__eq__", &cg::WebMercatorParams::operator==)
    .def("__ne__", &cg::WebMercatorParams::operator!=)
  ;

  class_<cg::LambertConformalConicParams>("GeoProjectionLCC2SP")
    .def(init<double, double, double, double, double, double, cg::Ellipsoid>(
      (arg("lat_0")=0.0, arg("lat_1")=-5.0, arg("lat_2")=5.0, arg("lon_0")=0.0, arg("x_0")=0.0, arg("y_0")=0.0, arg("ellps")=cg::Ellipsoid())))
    .def_readwrite("lat_0", &cg::LambertConformalConicParams::lat_0)
    .def_readwrite("lat_1", &cg::LambertConformalConicParams::lat_1)
    .def_readwrite("lat_2", &cg::LambertConformalConicParams::lat_2)
    .def_readwrite("lon_0", &cg::LambertConformalConicParams::lon_0)
    .def_readwrite("x_0", &cg::LambertConformalConicParams::x_0)
    .def_readwrite("y_0", &cg::LambertConformalConicParams::y_0)
    .def_readwrite("ellps", &cg::LambertConformalConicParams::ellps)
    .def("__eq__", &cg::LambertConformalConicParams::operator==)
    .def("__ne__", &cg::LambertConformalConicParams::operator!=)
  ;
}
