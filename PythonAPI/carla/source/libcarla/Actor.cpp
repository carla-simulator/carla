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
#include <carla/trafficmanager/TrafficManager.h>

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
  namespace ctm = carla::traffic_manager;

  class_<std::vector<int>>("vector_of_ints")
      .def(vector_indexing_suite<std::vector<int>>())
      .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Actor, boost::noncopyable, boost::shared_ptr<cc::Actor>>("Actor", no_init)
  // work-around, force return copy to resolve Actor instead of ActorState.
      .add_property("id", CALL_RETURNING_COPY(cc::Actor, GetId), "@DocString(Actor.id)")
      .add_property("type_id", CALL_RETURNING_COPY(cc::Actor, GetTypeId), "@DocString(Actor.type_id)")
      .add_property("parent", CALL_RETURNING_COPY(cc::Actor, GetParent), "@DocString(Actor.parent)")
      .add_property("semantic_tags", &GetSemanticTags, "@DocString(Actor.semantic_tags)")
      .add_property("is_alive", CALL_RETURNING_COPY(cc::Actor, IsAlive), "@DocString(Actor.is_alive)")
      .add_property("attributes", +[] (const cc::Actor &self) {
    boost::python::dict atttribute_dict;
    for (auto &&attribute_value : self.GetAttributes()) {
      atttribute_dict[attribute_value.GetId()] = attribute_value.GetValue();
    }
    return atttribute_dict;
  }, "@DocString(Actor.attributes)")
      .def("get_world", CALL_RETURNING_COPY(cc::Actor, GetWorld), "@DocString(Actor.get_world)")
      .def("get_location", &cc::Actor::GetLocation, "@DocString(Actor.get_location)")
      .def("get_transform", &cc::Actor::GetTransform, "@DocString(Actor.get_transform)")
      .def("get_velocity", &cc::Actor::GetVelocity, "@DocString(Actor.get_velocity)")
      .def("get_angular_velocity", &cc::Actor::GetAngularVelocity, "@DocString(Actor.get_angular_velocity)")
      .def("get_acceleration", &cc::Actor::GetAcceleration, "@DocString(Actor.get_acceleration)")
      .def("set_location", &cc::Actor::SetLocation, (arg("location")), "@DocString(Actor.set_location)")
      .def("set_transform", &cc::Actor::SetTransform, (arg("transform")), "@DocString(Actor.set_transform)")
      .def("set_velocity", &cc::Actor::SetVelocity, (arg("vector")), "@DocString(Actor.set_velocity)")
      .def("set_angular_velocity", &cc::Actor::SetAngularVelocity, (arg("vector")), "@DocString(Actor.set_angular_velocity)")
      .def("add_impulse", &cc::Actor::AddImpulse, (arg("vector")), "@DocString(Actor.add_impulse)")
      .def("set_simulate_physics", &cc::Actor::SetSimulatePhysics, (arg("enabled") = true), "@DocString(Actor.set_simulate_physics)")
      .def("destroy", CALL_WITHOUT_GIL(cc::Actor, Destroy), "@DocString(Actor.destroy)")
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
      .def("apply_control", &cc::Vehicle::ApplyControl, (arg("control")), "@DocString(Vehicle.apply_control)")
      .def("get_control", &cc::Vehicle::GetControl, "@DocString(Vehicle.get_control)")
      .def("set_light_state", &cc::Vehicle::SetLightState, (arg("light_state")), "@DocString(Vehicle.set_light_state)")
      .def("get_light_state", CONST_CALL_WITHOUT_GIL(cc::Vehicle, GetLightState), "@DocString(Vehicle.get_light_state)")
      .def("apply_physics_control", &cc::Vehicle::ApplyPhysicsControl, (arg("physics_control")), "@DocString(Vehicle.apply_physics_control)")
      .def("get_physics_control", CONST_CALL_WITHOUT_GIL(cc::Vehicle, GetPhysicsControl), "@DocString(Vehicle.get_physics_control)")
      .def("set_autopilot", CALL_WITHOUT_GIL_2(cc::Vehicle, SetAutopilot, bool, uint16_t), (arg("enabled") = true, arg("tm_port") = TM_DEFAULT_PORT), "@DocString(Vehicle.set_autopilot)")
      .def("get_speed_limit", &cc::Vehicle::GetSpeedLimit, "@DocString(Vehicle.get_speed_limit)")
      .def("get_traffic_light_state", &cc::Vehicle::GetTrafficLightState, "@DocString(Vehicle.get_traffic_light_state)")
      .def("is_at_traffic_light", &cc::Vehicle::IsAtTrafficLight, "@DocString(Vehicle.is_at_traffic_light)")
      .def("get_traffic_light", &cc::Vehicle::GetTrafficLight, "@DocString(Vehicle.get_traffic_light)")
      .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Walker, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Walker>>("Walker", no_init)
      .add_property("bounding_box", CALL_RETURNING_COPY(cc::Walker, GetBoundingBox))
      .def("apply_control", &ApplyControl<cr::WalkerControl>, (arg("control")), "@DocString(Walker.apply_control)")
      .def("apply_control", &ApplyControl<cr::WalkerBoneControl>, (arg("control")), "@DocString(Walker.apply_control)")
      .def("get_control", &cc::Walker::GetWalkerControl, "@DocString(Walker.get_control)")
      .def(self_ns::str(self_ns::self))
  ;

  class_<cc::WalkerAIController, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::WalkerAIController>>("WalkerAIController", no_init)
    .def("start", &cc::WalkerAIController::Start, "@DocString(WalkerAIController.start)")
    .def("stop", &cc::WalkerAIController::Stop, "@DocString(WalkerAIController.stop)")
    .def("go_to_location", &cc::WalkerAIController::GoToLocation, (arg("destination")), "@DocString(WalkerAIController.go_to_location)")
    .def("set_max_speed", &cc::WalkerAIController::SetMaxSpeed, (arg("speed")), "@DocString(WalkerAIController.set_max_speed)")
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::TrafficSign, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::TrafficSign>>(
      "TrafficSign",
      no_init)
      .add_property("trigger_volume", CALL_RETURNING_COPY(cc::TrafficSign, GetTriggerVolume), "@DocString(TrafficSign.trigger_volume)")
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
      .add_property("state", &cc::TrafficLight::GetState, "@DocString(TrafficLight.state)")
      .def("set_state", &cc::TrafficLight::SetState, (arg("state")), "@DocString(TrafficLight.set_state)")
      .def("get_state", &cc::TrafficLight::GetState, "@DocString(TrafficLight.get_state)")
      .def("set_green_time", &cc::TrafficLight::SetGreenTime, (arg("green_time")), "@DocString(TrafficLight.set_green_time)")
      .def("get_green_time", &cc::TrafficLight::GetGreenTime, "@DocString(TrafficLight.get_green_time)")
      .def("set_yellow_time", &cc::TrafficLight::SetYellowTime, (arg("yellow_time")), "@DocString(TrafficLight.set_yellow_time)")
      .def("get_yellow_time", &cc::TrafficLight::GetYellowTime, "@DocString(TrafficLight.get_yellow_time)")
      .def("set_red_time", &cc::TrafficLight::SetRedTime, (arg("red_time")), "@DocString(TrafficLight.set_red_time)")
      .def("get_red_time", &cc::TrafficLight::GetRedTime, "@DocString(TrafficLight.get_red_time)")
      .def("get_elapsed_time", &cc::TrafficLight::GetElapsedTime, "@DocString(TrafficLight.get_elapsed_time)")
      .def("freeze", &cc::TrafficLight::Freeze, (arg("freeze")), "@DocString(TrafficLight.freeze)")
      .def("is_frozen", &cc::TrafficLight::IsFrozen, "@DocString(TrafficLight.is_frozen)")
      .def("get_pole_index", &cc::TrafficLight::GetPoleIndex, "@DocString(TrafficLight.get_pole_index)")
      .def("get_group_traffic_lights", &GetGroupTrafficLights, "@DocString(TrafficLight.get_group_traffic_lights)")
      .def(self_ns::str(self_ns::self))
  ;
}
