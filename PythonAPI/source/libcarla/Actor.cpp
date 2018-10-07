// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Actor.h>
#include <carla/client/Vehicle.h>

#include <ostream>
#include <iostream>

namespace carla {
namespace client {

  std::ostream &operator<<(std::ostream &out, const Actor &actor) {
    out << "Actor(id=" << actor.GetId() << ", type=" << actor.GetTypeId() << ')';
    return out;
  }

} // namespace client
} // namespace carla

void export_actor() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cr = carla::rpc;

  class_<cc::Actor, boost::noncopyable, boost::shared_ptr<cc::Actor>>("Actor", no_init)
    // work-around, force return copy to resolve Actor instead of ActorState.
    .add_property("id", CALL_RETURNING_COPY(cc::Actor, GetId))
    .add_property("type_id", CALL_RETURNING_COPY(cc::Actor, GetTypeId))
    .add_property("is_alive", CALL_RETURNING_COPY(cc::Actor, IsAlive))
    .def("get_world", CALL_RETURNING_COPY(cc::Actor, GetWorld))
    .def("get_location", CONST_CALL_WITHOUT_GIL(cc::Actor, GetLocation))
    .def("get_transform", CONST_CALL_WITHOUT_GIL(cc::Actor, GetTransform))
    .def("set_location", &cc::Actor::SetLocation, (arg("location")))
    .def("set_transform", &cc::Actor::SetTransform, (arg("transform")))
    .def("destroy", CALL_WITHOUT_GIL(cc::Actor, Destroy))
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Vehicle, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Vehicle>>("Vehicle", no_init)
    .add_property("control", CALL_RETURNING_COPY(cc::Vehicle, GetControl))
    .def("apply_control", &cc::Vehicle::ApplyControl, (arg("control")))
    .def("set_autopilot", &cc::Vehicle::SetAutopilot, (arg("enabled")=true))
    .def(self_ns::str(self_ns::self))
  ;
}
