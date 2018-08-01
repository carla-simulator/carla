// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Control.h>

#include <boost/python.hpp>

#include <ostream>

namespace carla {
namespace rpc {

  std::ostream &operator<<(std::ostream &out, const VehicleControl &control) {
    auto boolalpha = [](bool b) { return b ? "True" : "False"; };
    out << "VehicleControl(throttle=" << control.throttle
        << ", steer=" << control.steer
        << ", brake=" << control.brake
        << ", hand_brake=" << boolalpha(control.hand_brake)
        << ", reverse=" << boolalpha(control.reverse) << ')';
    return out;
  }

} // namespace rpc
} // namespace carla

void export_control() {
  using namespace boost::python;
  namespace cc = carla::client;

  class_<cc::VehicleControl>("VehicleControl")
    .def(init<float, float, float, bool, bool>(
        (arg("throttle")=0.0f,
         arg("steer")=0.0f,
         arg("brake")=0.0f,
         arg("hand_brake")=false,
         arg("reverse")=false)))
    .def_readwrite("throttle", &cc::VehicleControl::throttle)
    .def_readwrite("steer", &cc::VehicleControl::steer)
    .def_readwrite("brake", &cc::VehicleControl::brake)
    .def_readwrite("hand_brake", &cc::VehicleControl::hand_brake)
    .def_readwrite("reverse", &cc::VehicleControl::reverse)
    .def(self_ns::str(self_ns::self))
  ;
}
