// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Actor.h>
#include <carla/client/Vehicle.h>

#include <boost/python.hpp>

#include <ostream>
#include <iostream>

namespace carla {
namespace client {

  std::ostream &operator<<(std::ostream &out, const Actor &actor) {
    out << "Actor(id=" << actor.GetId() << ", type=" << actor.GetTypeId() << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Vehicle &vehicle) {
    out << "Vehicle(id=" << vehicle.GetId() << ", type=" << vehicle.GetTypeId() << ')';
    return out;
  }

} // namespace client
} // namespace carla

void export_actor() {
  using namespace boost::python;
  namespace cc = carla::client;

  class_<cc::Actor, boost::noncopyable, boost::shared_ptr<cc::Actor>>("Actor", no_init)
    .add_property("id", &cc::Actor::GetId)
    .add_property("type_id", +[](const cc::Actor &self) -> std::string {
      return self.GetTypeId();
    })
    .def("get_world", &cc::Actor::GetWorld)
    .def("get_location", &cc::Actor::GetLocation)
    .def("get_transform", &cc::Actor::GetTransform)
    .def("set_location", &cc::Actor::SetLocation, (arg("location")))
    .def("set_transform", &cc::Actor::SetTransform, (arg("transform")))
    .def("destroy", &cc::Actor::Destroy)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Vehicle, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Vehicle>>("Vehicle", no_init)
    .def("apply_control", &cc::Vehicle::ApplyControl, (arg("control")))
    .def("set_autopilot", &cc::Vehicle::SetAutopilot, (arg("enabled")=true))
    .def(self_ns::str(self_ns::self))
  ;
}
