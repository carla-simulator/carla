// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/rpc/VehicleControl.h>
#include <carla/rpc/VehiclePhysicsControl.h>
#include <carla/rpc/WheelPhysicsControl.h>
#include <carla/rpc/WalkerControl.h>

#include <ostream>

namespace carla {
namespace rpc {

  static auto boolalpha(bool b) {
    return b ? "True" : "False";
  };

  std::ostream &operator<<(std::ostream &out, const VehicleControl &control) {
    out << "VehicleControl(throttle=" << control.throttle
        << ", steer=" << control.steer
        << ", brake=" << control.brake
        << ", hand_brake=" << boolalpha(control.hand_brake)
        << ", reverse=" << boolalpha(control.reverse)
        << ", manual_gear_shift=" << boolalpha(control.manual_gear_shift)
        << ", gear=" << control.gear << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const WalkerControl &control) {
    out << "WalkerControl(direction=" << control.direction
        << ", speed=" << control.speed
        << ", jump=" << boolalpha(control.jump) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const WheelPhysicsControl &control) {
    out << "WheelPhysicsControl(tire_friction=" << control.tire_friction 
        << ", torque=" << control.torque
        << ", mass=" << control.mass
        << ", disable_steering=" << control.disable_steering << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const VehiclePhysicsControl &control) {
    out << "VehiclePhysicsControl(torque_curve=" << control.torque_curve
    << ", max_rpm=" << control.max_rpm
    << ", moi=" << control.moi
    << ", damping_rate_full_throttle=" << control.damping_rate_full_throttle
    << ", damping_rate_zero_throttle_clutch_engaged=" << control.damping_rate_zero_throttle_clutch_engaged
    << ", damping_rate_zero_throttle_clutch_disengaged=" << control.damping_rate_zero_throttle_clutch_disengaged 
    << ", use_gear_autobox=" << control.use_gear_autobox
    << ", gear_switch_time=" << control.gear_switch_time
    << ", clutch_strength=" << control.clutch_strength
    << ", mass=" << control.mass
    << ", drag_coefficient=" << control.drag_coefficient
    << ", inertia_tensor_scale=" << control.inertia_tensor_scale 
    << ", wheels=" << control.wheels << ')';
    return out;
  }
} // namespace rpc
} // namespace carla



static auto GetWheels(const carla::rpc::VehiclePhysicsControl &self) {
  const auto &wheels = self.GetWheels();
  boost::python::object get_iter = boost::python::iterator<std::vector<carla::rpc::WheelPhysicsControl>>();
  boost::python::object iter = get_iter(wheels);
  return boost::python::list(wheels);
}

static void SetWheels(carla::rpc::VehiclePhysicsControl &self, const boost::python::list &list) {  
  std::vector<carla::rpc::WheelPhysicsControl> wheels;
  auto length = boost::python::len(list);
  for (auto i = 0u; i < length; ++i) {
    wheels.push_back(boost::python::extract<carla::rpc::WheelPhysicsControl &>(list[i]));
  }
  self.wheels = wheels;
}

static auto GetTorqueCurve(const carla::rpc::VehiclePhysicsControl &self) {
  const auto &torque_curve = self.GetTorqueCurve();
  boost::python::object get_iter = boost::python::iterator<std::vector<carla::geom::Location>>();
  boost::python::object iter = get_iter(torque_curve);
  return boost::python::list(torque_curve);
}

static void SetTorqueCurve(carla::rpc::VehiclePhysicsControl &self, const boost::python::list &list) {  
  std::vector<carla::geom::Location> torque_curve;
  auto length = boost::python::len(list);
  for (auto i = 0u; i < length; ++i) {
    torque_curve.push_back(boost::python::extract<carla::geom::Location &>(list[i]));
  }
  self.torque_curve = torque_curve;
}

static auto GetSteeringCurve(const carla::rpc::VehiclePhysicsControl &self) {
  const auto &steering_curve = self.GetSteeringCurve();
  boost::python::object get_iter = boost::python::iterator<std::vector<carla::geom::Location>>();
  boost::python::object iter = get_iter(steering_curve);
  return boost::python::list(steering_curve);
}

static void SetSteeringCurve(carla::rpc::VehiclePhysicsControl &self, const boost::python::list &list) {  
  std::vector<carla::geom::Location> steering_curve;
  auto length = boost::python::len(list);
  for (auto i = 0u; i < length; ++i) {
    steering_curve.push_back(boost::python::extract<carla::geom::Location &>(list[i]));
  }
  self.steering_curve = steering_curve;
}

void export_control() {
  using namespace boost::python;
  namespace cr = carla::rpc;
  namespace cg = carla::geom;

  class_<cr::VehicleControl>("VehicleControl")
    .def(init<float, float, float, bool, bool, bool, int>(
        (arg("throttle")=0.0f,
         arg("steer")=0.0f,
         arg("brake")=0.0f,
         arg("hand_brake")=false,
         arg("reverse")=false,
         arg("manual_gear_shift")=false,
         arg("gear")=0)))
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
        (arg("direction")=cg::Vector3D{1.0f, 0.0f, 0.0f},
         arg("speed")=0.0f,
         arg("jump")=false)))
    .def_readwrite("direction", &cr::WalkerControl::direction)
    .def_readwrite("speed", &cr::WalkerControl::speed)
    .def_readwrite("jump", &cr::WalkerControl::jump)
    .def("__eq__", &cr::WalkerControl::operator==)
    .def("__ne__", &cr::WalkerControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<std::vector<carla::geom::Location>>("vector_of_locations")
    .def(vector_indexing_suite<std::vector<cg::Location>>())
    .def(self_ns::str(self_ns::self))
  ;

  class_<std::vector<cr::WheelPhysicsControl>>("vector_of_wheels")
    .def(vector_indexing_suite<std::vector<cr::WheelPhysicsControl>>())
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::WheelPhysicsControl>("WheelPhysicsControl")
    .def(init<float, float, float, bool>(
        (arg("tire_friction")=0.0f,
         arg("torque")=0.0f,
         arg("mass")=0.0f,
         arg("disable_steering")=false)))
    .def_readwrite("tire_friction", &cr::WheelPhysicsControl::tire_friction)
    // .def("__eq__", &cr::WheelPhysicsControl::operator==)
    // .def("__ne__", &cr::WheelPhysicsControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::VehiclePhysicsControl>("VehiclePhysicsControl")
    .def(init<std::vector<cg::Location>, float, float, float, float, float,
              bool, float, float,
              float, float, cg::Vector3D,
              const std::vector<cg::Location>&,
              std::vector<cr::WheelPhysicsControl>&
            >
       ((arg("torque_curve")=std::vector<cg::Location>(),
         arg("max_rpm")=0.0f,
         arg("moi")=0.0f,
         arg("damping_rate_full_throttle")=0.0f,
         arg("damping_rate_zero_throttle_clutch_engaged")=0.0f,
         arg("damping_rate_zero_throttle_clutch_disengaged")=0.0f,
         arg("use_gear_autobox")=true,
         arg("gear_switch_time")=0.0f,
         arg("clutch_strength")=0.0f,
         arg("mass")=0.0f,
         arg("drag_coefficient")=0.0f,
         arg("inertia_tensor_scale")=cg::Vector3D{1.0f, 0.0f, 0.0f},
         arg("steering_curve")=std::vector<cg::Location>(),
         arg("wheels")=std::vector<cr::WheelPhysicsControl>()
         )))
    .add_property("torque_curve", &GetTorqueCurve, &SetTorqueCurve)
    .add_property("steering_curve", &GetSteeringCurve, &SetSteeringCurve)
    .def_readwrite("max_rpm", &cr::VehiclePhysicsControl::max_rpm)
    .def_readwrite("moi", &cr::VehiclePhysicsControl::moi)
    .def_readwrite("damping_rate_full_throttle", &cr::VehiclePhysicsControl::damping_rate_full_throttle)
    .def_readwrite("damping_rate_zero_throttle_clutch_engaged", &cr::VehiclePhysicsControl::damping_rate_zero_throttle_clutch_engaged)
    .def_readwrite("damping_rate_zero_throttle_clutch_disengaged", &cr::VehiclePhysicsControl::damping_rate_zero_throttle_clutch_disengaged)
    
    .def_readwrite("use_gear_autobox", &cr::VehiclePhysicsControl::use_gear_autobox)
    .def_readwrite("gear_switch_time", &cr::VehiclePhysicsControl::gear_switch_time)
    .def_readwrite("clutch_strength", &cr::VehiclePhysicsControl::clutch_strength)
    
    .def_readwrite("mass", &cr::VehiclePhysicsControl::mass)
    .def_readwrite("drag_coefficient", &cr::VehiclePhysicsControl::drag_coefficient)
    .def_readwrite("inertia_tensor_scale", &cr::VehiclePhysicsControl::inertia_tensor_scale)
    
    .add_property("wheels", &GetWheels, &SetWheels)

    // [TODO]
    // .def("__eq__", &cr::VehiclePhysicsControl::operator==)
    // [TODO]
    // .def("__ne__", &cr::VehiclePhysicsControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;
}
