// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Actor.h>
#include <carla/client/TrafficLight.h>
#include <carla/client/Vehicle.h>
#include <carla/client/Walker.h>
#include <carla/client/WalkerAIController.h>
#include <carla/rpc/TrafficLightState.h>

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
  boost::python::object get_iter = boost::python::iterator<std::vector<int>>();
  boost::python::object iter = get_iter(tags);
  return boost::python::list(iter);
}

static auto GetGroupTrafficLights(carla::client::TrafficLight &self) {
  namespace py = boost::python;
  auto values = self.GetGroupTrafficLights();
  py::list result;
  for (auto value : values) {
    result.append(value);
  }
  return result;
}

template <typename ControlT>
static void ApplyControl(carla::client::Walker &self, const ControlT &control) {
  self.ApplyControl(control);
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
      .add_property("parent", CALL_RETURNING_COPY(cc::Actor, GetParent))
      .add_property("semantic_tags", &GetSemanticTags)
      .add_property("is_alive", CALL_RETURNING_COPY(cc::Actor, IsAlive))
      .add_property("attributes", +[] (const cc::Actor &self) {
    boost::python::dict atttribute_dict;
    for (auto &&attribute_value : self.GetAttributes()) {
      atttribute_dict[attribute_value.GetId()] = attribute_value.GetValue();
    }
    return atttribute_dict;
  })
      .def("get_world", CALL_RETURNING_COPY(cc::Actor, GetWorld))
      .def("get_location", &cc::Actor::GetLocation)
      .def("get_transform", &cc::Actor::GetTransform)
      .def("get_velocity", &cc::Actor::GetVelocity)
      .def("get_angular_velocity", &cc::Actor::GetAngularVelocity)
      .def("get_acceleration", &cc::Actor::GetAcceleration)
      .def("set_location", &cc::Actor::SetLocation, (arg("location")))
      .def("set_transform", &cc::Actor::SetTransform, (arg("transform")))
      .def("set_velocity", &cc::Actor::SetVelocity, (arg("vector")))
      .def("set_angular_velocity", &cc::Actor::SetAngularVelocity, (arg("vector")))
      .def("add_impulse", &cc::Actor::AddImpulse, (arg("vector")))
      .def("set_simulate_physics", &cc::Actor::SetSimulatePhysics, (arg("enabled") = true))
      .def("destroy", CALL_WITHOUT_GIL(cc::Actor, Destroy))
      .def(self_ns::str(self_ns::self))
  ;

  enum_<cr::VehicleLightState::LightState>("VehicleLightState")
    .value("NONE", cr::VehicleLightState::LightState::None) // None is reserved in Python3
    .value("Position", cr::VehicleLightState::LightState::Position)
    .value("LowBeam", cr::VehicleLightState::LightState::LowBeam)
    .value("HighBeam", cr::VehicleLightState::LightState::HighBeam)
    .value("Brake", cr::VehicleLightState::LightState::Brake)
    .value("RightBlinker", cr::VehicleLightState::LightState::RightBlinker)
    .value("LeftBlinker", cr::VehicleLightState::LightState::LeftBlinker)
    .value("Reverse", cr::VehicleLightState::LightState::Reverse)
    .value("Fog", cr::VehicleLightState::LightState::Fog)
    .value("Interior", cr::VehicleLightState::LightState::Interior)
    .value("Special1", cr::VehicleLightState::LightState::Special1)
    .value("Special2", cr::VehicleLightState::LightState::Special2)
    .value("All", cr::VehicleLightState::LightState::All)
  ;

  class_<cc::Vehicle, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Vehicle>>("Vehicle",
      no_init)
      .add_property("bounding_box", CALL_RETURNING_COPY(cc::Vehicle, GetBoundingBox))
      .def("apply_control", &cc::Vehicle::ApplyControl, (arg("control")))
      .def("get_control", &cc::Vehicle::GetControl)
      .def("set_light_state", &cc::Vehicle::SetLightState, (arg("light_state")))
      .def("get_light_state", CONST_CALL_WITHOUT_GIL(cc::Vehicle, GetLightState))
      .def("apply_physics_control", &cc::Vehicle::ApplyPhysicsControl, (arg("physics_control")))
      .def("get_physics_control", CONST_CALL_WITHOUT_GIL(cc::Vehicle, GetPhysicsControl))
      .def("set_autopilot", &cc::Vehicle::SetAutopilot, (arg("enabled") = true))
      .def("get_speed_limit", &cc::Vehicle::GetSpeedLimit)
      .def("get_traffic_light_state", &cc::Vehicle::GetTrafficLightState)
      .def("is_at_traffic_light", &cc::Vehicle::IsAtTrafficLight)
      .def("get_traffic_light", &cc::Vehicle::GetTrafficLight)
      .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Walker, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Walker>>("Walker", no_init)
      .add_property("bounding_box", CALL_RETURNING_COPY(cc::Walker, GetBoundingBox))
      .def("apply_control", &ApplyControl<cr::WalkerControl>, (arg("control")))
      .def("apply_control", &ApplyControl<cr::WalkerBoneControl>, (arg("control")))
      .def("get_control", &cc::Walker::GetWalkerControl)
      .def(self_ns::str(self_ns::self))
  ;

  class_<cc::WalkerAIController, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::WalkerAIController>>("WalkerAIController", no_init)
    .def("start", &cc::WalkerAIController::Start)
    .def("stop", &cc::WalkerAIController::Stop)
    .def("go_to_location", &cc::WalkerAIController::GoToLocation, (arg("destination")))
    .def("set_max_speed", &cc::WalkerAIController::SetMaxSpeed, (arg("speed")))
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::TrafficSign, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::TrafficSign>>(
      "TrafficSign",
      no_init)
      .add_property("trigger_volume", CALL_RETURNING_COPY(cc::TrafficSign, GetTriggerVolume))
  ;

  enum_<cr::TrafficLightState>("TrafficLightState")
      .value("Red", cr::TrafficLightState::Red)
      .value("Yellow", cr::TrafficLightState::Yellow)
      .value("Green", cr::TrafficLightState::Green)
      .value("Off", cr::TrafficLightState::Off)
      .value("Unknown", cr::TrafficLightState::Unknown)
  ;

  class_<cc::TrafficLight, bases<cc::TrafficSign>, boost::noncopyable, boost::shared_ptr<cc::TrafficLight>>(
      "TrafficLight",
      no_init)
      .add_property("state", &cc::TrafficLight::GetState)
      .def("set_state", &cc::TrafficLight::SetState, (arg("state")))
      .def("get_state", &cc::TrafficLight::GetState)
      .def("set_green_time", &cc::TrafficLight::SetGreenTime, (arg("green_time")))
      .def("get_green_time", &cc::TrafficLight::GetGreenTime)
      .def("set_yellow_time", &cc::TrafficLight::SetYellowTime, (arg("yellow_time")))
      .def("get_yellow_time", &cc::TrafficLight::GetYellowTime)
      .def("set_red_time", &cc::TrafficLight::SetRedTime, (arg("red_time")))
      .def("get_red_time", &cc::TrafficLight::GetRedTime)
      .def("get_elapsed_time", &cc::TrafficLight::GetElapsedTime)
      .def("freeze", &cc::TrafficLight::Freeze, (arg("freeze")))
      .def("is_frozen", &cc::TrafficLight::IsFrozen)
      .def("get_pole_index", &cc::TrafficLight::GetPoleIndex)
      .def("get_group_traffic_lights", &GetGroupTrafficLights)
      .def(self_ns::str(self_ns::self))
  ;
}
