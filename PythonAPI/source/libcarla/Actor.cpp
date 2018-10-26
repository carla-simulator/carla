// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Actor.h>
#include <carla/client/TrafficLight.h>
#include <carla/client/Vehicle.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

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

static auto GetSemanticTags(const carla::client::Actor &self) {
  const auto &tags = self.GetSemanticTags();
  return std::vector<int>(tags.begin(), tags.end());
}

void export_actor() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cr = carla::rpc;

  class_<std::vector<int>>("vector_of_ints")
    .def(vector_indexing_suite<std::vector<int>>())
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Actor, boost::noncopyable, boost::shared_ptr<cc::Actor>>("Actor", no_init)
    // work-around, force return copy to resolve Actor instead of ActorState.
    .add_property("id", CALL_RETURNING_COPY(cc::Actor, GetId))
    .add_property("type_id", CALL_RETURNING_COPY(cc::Actor, GetTypeId))
    .add_property("semantic_tags", &GetSemanticTags)
    .add_property("is_alive", CALL_RETURNING_COPY(cc::Actor, IsAlive))
    .def("get_world", CALL_RETURNING_COPY(cc::Actor, GetWorld))
    .def("get_location", &cc::Actor::GetLocation)
    .def("get_transform", &cc::Actor::GetTransform)
    .def("get_velocity", &cc::Actor::GetVelocity)
    .def("get_acceleration", &cc::Actor::GetAcceleration)
    .def("set_location", &cc::Actor::SetLocation, (arg("location")))
    .def("set_transform", &cc::Actor::SetTransform, (arg("transform")))
    .def("set_simulate_physics", &cc::Actor::SetSimulatePhysics, (arg("enabled")=true))
    .def("destroy", CALL_WITHOUT_GIL(cc::Actor, Destroy))
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Vehicle, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Vehicle>>("Vehicle", no_init)
    .add_property("bounding_box", CALL_RETURNING_COPY(cc::Vehicle, GetBoundingBox))
    .add_property("control", CALL_RETURNING_COPY(cc::Vehicle, GetControl))
    .def("apply_control", &cc::Vehicle::ApplyControl, (arg("control")))
    .def("set_autopilot", &cc::Vehicle::SetAutopilot, (arg("enabled")=true))
    .def(self_ns::str(self_ns::self))
  ;

  enum_<cc::TrafficLightState>("TrafficLightState")
    .value("Unknown", cc::TrafficLightState::Unknown)
    .value("Red", cc::TrafficLightState::Red)
    .value("Yellow", cc::TrafficLightState::Yellow)
    .value("Green", cc::TrafficLightState::Green)
  ;

  class_<cc::TrafficLight, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::TrafficLight>>("TrafficLight", no_init)
    .add_property("state", &cc::TrafficLight::GetState)
    .def(self_ns::str(self_ns::self))
  ;
}
