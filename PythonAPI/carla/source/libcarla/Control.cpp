// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/rpc/VehicleControl.h>
#include <carla/rpc/VehiclePhysicsControl.h>
#include <carla/rpc/WheelPhysicsControl.h>
#include <carla/rpc/WalkerControl.h>
#include <carla/rpc/WalkerBoneControl.h>

#include <ostream>

namespace carla {
namespace rpc {

  static auto boolalpha(bool b) {
    return b ? "True" : "False";
  }

  std::ostream &operator<<(std::ostream &out, const VehicleControl &control) {
    out << "VehicleControl(throttle=" << std::to_string(control.throttle)
        << ", steer=" << std::to_string(control.steer)
        << ", brake=" << std::to_string(control.brake)
        << ", hand_brake=" << boolalpha(control.hand_brake)
        << ", reverse=" << boolalpha(control.reverse)
        << ", manual_gear_shift=" << boolalpha(control.manual_gear_shift)
        << ", gear=" << std::to_string(control.gear) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const WalkerControl &control) {
    out << "WalkerControl(direction=" << control.direction
        << ", speed=" << std::to_string(control.speed)
        << ", jump=" << boolalpha(control.jump) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const WalkerBoneControl &control) {
    out << "WalkerBoneControl(bone_transforms(";
    for (auto bone_transform : control.bone_transforms) {
      out << "(name="  << bone_transform.first
          << ", transform=" << bone_transform.second << ')';
    }
    out << "))";
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const WheelPhysicsControl &control) {
    out << "WheelPhysicsControl(tire_friction=" << std::to_string(control.tire_friction)
        << ", damping_rate=" << std::to_string(control.damping_rate)
        << ", max_steer_angle=" << std::to_string(control.max_steer_angle)
        << ", radius=" << std::to_string(control.radius)
        << ", position=" << control.position << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const VehiclePhysicsControl &control) {
    out << "VehiclePhysicsControl(torque_curve=" << control.torque_curve
    << ", max_rpm=" << std::to_string(control.max_rpm)
    << ", moi=" << std::to_string(control.moi)
    << ", damping_rate_full_throttle=" << std::to_string(control.damping_rate_full_throttle)
    << ", damping_rate_zero_throttle_clutch_engaged=" << std::to_string(control.damping_rate_zero_throttle_clutch_engaged)
    << ", damping_rate_zero_throttle_clutch_disengaged=" << std::to_string(control.damping_rate_zero_throttle_clutch_disengaged)
    << ", use_gear_autobox=" << boolalpha(control.use_gear_autobox)
    << ", gear_switch_time=" << std::to_string(control.gear_switch_time)
    << ", clutch_strength=" << std::to_string(control.clutch_strength)
    << ", mass=" << std::to_string(control.mass)
    << ", drag_coefficient=" << std::to_string(control.drag_coefficient)
    << ", center_of_mass=" << control.center_of_mass
    << ", steering_curve=" << control.steering_curve
    << ", wheels=" << control.wheels << ')';
    return out;
  }
} // namespace rpc
} // namespace carla

namespace bp = boost::python;
namespace cr = carla::rpc;


static auto GetVectorOfVector2DFromList(const bp::list &list) {
  std::vector<carla::geom::Vector2D> v;

  auto length = bp::len(list);
  v.reserve(static_cast<size_t>(length));
  for (auto i = 0u; i < length; ++i) {
    bp::extract<carla::geom::Vector2D> ext(list[i]);
    if (ext.check()) {
      v.push_back(ext);
    } else {
      v.push_back(carla::geom::Vector2D{
        bp::extract<float>(list[i][0u]),
        bp::extract<float>(list[i][1u])});
    }
  }
  return v;
}

static auto GetVectorOfBoneTransformFromList(const bp::list &list) {
  std::vector<cr::BoneTransformData> v;

  auto length = bp::len(list);
  v.reserve(static_cast<size_t>(length));
  for (auto i = 0u; i < length; ++i) {
    bp::extract<cr::BoneTransformData> ext(list[i]);
    if (ext.check()) {
      v.push_back(ext);
    } else {
      v.push_back(cr::BoneTransformData{
        bp::extract<std::string>(list[i][0u]),
        bp::extract<carla::geom::Transform>(list[i][1u])});
    }
  }
  return v;
}

static auto GetWheels(const cr::VehiclePhysicsControl &self) {
  const auto &wheels = self.GetWheels();
  bp::object get_iter = bp::iterator<std::vector<cr::WheelPhysicsControl>>();
  bp::object iter = get_iter(wheels);
  return bp::list(iter);
}

static void SetWheels(cr::VehiclePhysicsControl &self, const bp::list &list) {
  std::vector<cr::WheelPhysicsControl> wheels;
  auto length = bp::len(list);
  for (auto i = 0u; i < length; ++i) {
    wheels.push_back(bp::extract<cr::WheelPhysicsControl &>(list[i]));
  }
  self.wheels = wheels;
}

static auto GetTorqueCurve(const cr::VehiclePhysicsControl &self) {
  const std::vector<carla::geom::Vector2D> &torque_curve = self.GetTorqueCurve();
  bp::object get_iter = bp::iterator<const std::vector<carla::geom::Vector2D>>();
  bp::object iter = get_iter(torque_curve);
  return bp::list(iter);
}

static void SetTorqueCurve(cr::VehiclePhysicsControl &self, const bp::list &list) {
  self.torque_curve = GetVectorOfVector2DFromList(list);
}

static auto GetSteeringCurve(const cr::VehiclePhysicsControl &self) {
  const std::vector<carla::geom::Vector2D> &steering_curve = self.GetSteeringCurve();
  bp::object get_iter = bp::iterator<const std::vector<carla::geom::Vector2D>>();
  bp::object iter = get_iter(steering_curve);
  return bp::list(iter);
}

static void SetSteeringCurve(cr::VehiclePhysicsControl &self, const bp::list &list) {
  self.steering_curve = GetVectorOfVector2DFromList(list);
}

bp::object VehiclePhysicsControl_init(bp::tuple args, bp::dict kwargs) {
  // Args names
  const uint32_t NUM_ARGUMENTS = 16;
  const char *args_names[NUM_ARGUMENTS] = {
    "torque_curve",
    "max_rpm",
    "moi",
    "damping_rate_full_throttle",
    "damping_rate_zero_throttle_clutch_engaged",
    "damping_rate_zero_throttle_clutch_disengaged",

    "use_gear_autobox",
    "gear_switch_time",
    "clutch_strength",

    "mass",
    "drag_coefficient",

    "center_of_mass",
    "steering_curve",
    "wheels"
  };

  bp::object self = args[0];
  args = bp::tuple(args.slice(1, bp::_));

  auto res = self.attr("__init__")();
  if (len(args) > 0) {
    for (unsigned int i = 0; i < len(args); ++i) {
      self.attr(args_names[i]) = args[i];
    }
  }

  for (unsigned int i = 0; i < NUM_ARGUMENTS; ++i) {
    if (kwargs.contains(args_names[i])) {
      self.attr(args_names[i]) = kwargs[args_names[i]];
    }
  }

  return res;
}

static auto GetBonesTransform(const cr::WalkerBoneControl &self) {
  const std::vector<cr::BoneTransformData> &bone_transform_data = self.bone_transforms;
  bp::object get_iter =
      bp::iterator<const std::vector<cr::BoneTransformData>>();
  bp::object iter = get_iter(bone_transform_data);
  return bp::list(iter);
}

static void SetBonesTransform(cr::WalkerBoneControl &self, const bp::list &list) {
  self.bone_transforms = GetVectorOfBoneTransformFromList(list);
}

bp::object WalkerBoneControl_init(bp::tuple args, bp::dict kwargs) {
  // Args names
  const uint32_t NUM_ARGUMENTS = 1;
  const char *args_names[NUM_ARGUMENTS] = {
    "bone_transforms"
  };

  bp::object self = args[0];
  args = bp::tuple(args.slice(1, bp::_));

  auto res = self.attr("__init__")();
  if (len(args) > 0) {
    for (unsigned int i = 0; i < len(args); ++i) {
      self.attr(args_names[i]) = args[i];
    }
  }

  for (unsigned int i = 0; i < NUM_ARGUMENTS; ++i) {
    if (kwargs.contains(args_names[i])) {
      self.attr(args_names[i]) = kwargs[args_names[i]];
    }
  }

  return res;
}

void export_control() {
  using namespace boost::python;
  namespace cg = carla::geom;

  class_<cr::VehicleControl>("VehicleControl")
    .def(init<float, float, float, bool, bool, bool, int>(
      (arg("throttle") = 0.0f,
      arg("steer") = 0.0f,
      arg("brake") = 0.0f,
      arg("hand_brake") = false,
      arg("reverse") = false,
      arg("manual_gear_shift") = false,
      arg("gear") = 0)))
    .def_readwrite("throttle", &cr::VehicleControl::throttle)
    .def_readwrite("steer", &cr::VehicleControl::steer)
    .def_readwrite("brake", &cr::VehicleControl::brake)
    .def_readwrite("hand_brake", &cr::VehicleControl::hand_brake)
    .def_readwrite("reverse", &cr::VehicleControl::reverse)
    .def_readwrite("manual_gear_shift", &cr::VehicleControl::manual_gear_shift)
    .def_readwrite("gear", &cr::VehicleControl::gear)
    .def("__eq__", &cr::VehicleControl::operator==)
    .def("__ne__", &cr::VehicleControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::WalkerControl>("WalkerControl")
    .def(init<cg::Vector3D, float, bool>(
       (arg("direction") = cg::Vector3D{1.0f, 0.0f, 0.0f},
       arg("speed") = 0.0f,
       arg("jump") = false)))
    .def_readwrite("direction", &cr::WalkerControl::direction)
    .def_readwrite("speed", &cr::WalkerControl::speed)
    .def_readwrite("jump", &cr::WalkerControl::jump)
    .def("__eq__", &cr::WalkerControl::operator==)
    .def("__ne__", &cr::WalkerControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::WalkerBoneControl>("WalkerBoneControl")
    .def("__init__", raw_function(WalkerBoneControl_init))
    .def(init<>())
    .add_property("bone_transforms", &GetBonesTransform, &SetBonesTransform)
    .def(self_ns::str(self_ns::self))
  ;

  class_<std::vector<cr::WheelPhysicsControl>>("vector_of_wheels")
    .def(boost::python::vector_indexing_suite<std::vector<cr::WheelPhysicsControl>>())
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::WheelPhysicsControl>("WheelPhysicsControl")
    .def(init<float, float, float, float, cg::Vector3D>(
        (arg("tire_friction")=2.0f,
         arg("damping_rate")=0.25f,
         arg("max_steer_angle")=70.0f,
         arg("radius")=30.0f,
         arg("position")=cg::Vector3D{0.0f, 0.0f, 0.0f})))
    .def_readwrite("tire_friction", &cr::WheelPhysicsControl::tire_friction)
    .def_readwrite("damping_rate", &cr::WheelPhysicsControl::damping_rate)
    .def_readwrite("max_steer_angle", &cr::WheelPhysicsControl::max_steer_angle)
    .def_readwrite("radius", &cr::WheelPhysicsControl::radius)
    .def_readwrite("position", &cr::WheelPhysicsControl::position)
    .def("__eq__", &cr::WheelPhysicsControl::operator==)
    .def("__ne__", &cr::WheelPhysicsControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::VehiclePhysicsControl>("VehiclePhysicsControl", no_init)
    .def("__init__", raw_function(VehiclePhysicsControl_init))
    .def(init<>())
    .add_property("torque_curve", &GetTorqueCurve, &SetTorqueCurve)
    .def_readwrite("max_rpm", &cr::VehiclePhysicsControl::max_rpm)
    .def_readwrite("moi", &cr::VehiclePhysicsControl::moi)
    .def_readwrite("damping_rate_full_throttle",
        &cr::VehiclePhysicsControl::damping_rate_full_throttle)
    .def_readwrite("damping_rate_zero_throttle_clutch_engaged",
        &cr::VehiclePhysicsControl::damping_rate_zero_throttle_clutch_engaged)
    .def_readwrite("damping_rate_zero_throttle_clutch_disengaged",
        &cr::VehiclePhysicsControl::damping_rate_zero_throttle_clutch_disengaged)
    .def_readwrite("use_gear_autobox", &cr::VehiclePhysicsControl::use_gear_autobox)
    .def_readwrite("gear_switch_time", &cr::VehiclePhysicsControl::gear_switch_time)
    .def_readwrite("clutch_strength", &cr::VehiclePhysicsControl::clutch_strength)
    .def_readwrite("mass", &cr::VehiclePhysicsControl::mass)
    .def_readwrite("drag_coefficient", &cr::VehiclePhysicsControl::drag_coefficient)
    .def_readwrite("center_of_mass", &cr::VehiclePhysicsControl::center_of_mass)
    .add_property("steering_curve", &GetSteeringCurve, &SetSteeringCurve)
    .add_property("wheels", &GetWheels, &SetWheels)
    .def("__eq__", &cr::VehiclePhysicsControl::operator==)
    .def("__ne__", &cr::VehiclePhysicsControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;
}
