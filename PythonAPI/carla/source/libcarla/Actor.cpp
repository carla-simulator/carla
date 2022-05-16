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

namespace ctm = carla::traffic_manager;

namespace carla {
namespace client {

  std::ostream &operator<<(std::ostream &out, const Actor &actor) {
    out << "Actor(id=" << actor.GetId() << ", type=" << actor.GetTypeId() << ')';
    return out;
  }

} // namespace client
} // namespace carla

template<class T>
boost::python::list StdVectorToPyList(const std::vector<T> &vec) {
  boost::python::list l;
  for (auto &e : vec) {
    l.append(e);
  }
  return l;
}

static boost::python::list GetSemanticTags(const carla::client::Actor &self) {
  const std::vector<uint8_t> &tags = self.GetSemanticTags();
  return StdVectorToPyList(tags);
}

static void AddActorImpulse(carla::client::Actor &self,
    const carla::geom::Vector3D &impulse) {
  self.AddImpulse(impulse);
}

static void AddActorForce(carla::client::Actor &self,
    const carla::geom::Vector3D &force) {
  self.AddForce(force);
}

static auto GetGroupTrafficLights(carla::client::TrafficLight &self) {
  auto values = self.GetGroupTrafficLights();
  return StdVectorToPyList(values);
}

template <typename ControlT>
static void ApplyControl(carla::client::Walker &self, const ControlT &control) {
  self.ApplyControl(control);
}

static auto GetLightBoxes(const carla::client::TrafficLight &self) {
  boost::python::list result;
  for (const auto &bb : self.GetLightBoxes()) {
    result.append(bb);
  }
  return result;
}

void export_actor() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cr = carla::rpc;
  namespace ctm = carla::traffic_manager;

  enum_<cr::ActorState>("ActorState")
    .value("Invalid", cr::ActorState::Invalid)
    .value("Active", cr::ActorState::Active)
    .value("Dormant", cr::ActorState::Dormant)
  ;

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
      .add_property("actor_state", CALL_WITHOUT_GIL(cc::Actor, GetActorState))
      .add_property("is_alive", CALL_WITHOUT_GIL(cc::Actor, IsAlive))
      .add_property("is_dormant", CALL_WITHOUT_GIL(cc::Actor, IsDormant))
      .add_property("is_active", CALL_WITHOUT_GIL(cc::Actor, IsActive))
      .add_property("attributes", +[] (const cc::Actor &self) {
        boost::python::dict attribute_dict;
        for (auto &&attribute_value : self.GetAttributes()) {
          attribute_dict[attribute_value.GetId()] = attribute_value.GetValue();
        }
        return attribute_dict;
      })
      .add_property("bounding_box", CALL_RETURNING_COPY(cc::Actor, GetBoundingBox))
      .def("get_world", CALL_RETURNING_COPY(cc::Actor, GetWorld))
      .def("get_location", &cc::Actor::GetLocation)
      .def("get_transform", &cc::Actor::GetTransform)
      .def("get_velocity", &cc::Actor::GetVelocity)
      .def("get_angular_velocity", &cc::Actor::GetAngularVelocity)
      .def("get_acceleration", &cc::Actor::GetAcceleration)
      .def("set_location", &cc::Actor::SetLocation, (arg("location")))
      .def("set_transform", &cc::Actor::SetTransform, (arg("transform")))
      .def("set_target_velocity", &cc::Actor::SetTargetVelocity, (arg("velocity")))
      .def("set_target_angular_velocity", &cc::Actor::SetTargetAngularVelocity, (arg("angular_velocity")))
      .def("enable_constant_velocity", &cc::Actor::EnableConstantVelocity, (arg("velocity")))
      .def("disable_constant_velocity", &cc::Actor::DisableConstantVelocity)
      .def("add_impulse", &AddActorImpulse, (arg("impulse")))
      .def("add_force", &AddActorForce, (arg("force")))
      .def("add_angular_impulse", &cc::Actor::AddAngularImpulse, (arg("angular_impulse")))
      .def("add_torque", &cc::Actor::AddTorque, (arg("torque")))
      .def("set_simulate_physics", &cc::Actor::SetSimulatePhysics, (arg("enabled") = true))
      .def("set_enable_gravity", &cc::Actor::SetEnableGravity, (arg("enabled") = true))
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

  enum_<cr::VehicleWheelLocation>("VehicleWheelLocation")
    .value("FL_Wheel", cr::VehicleWheelLocation::FL_Wheel)
    .value("FR_Wheel", cr::VehicleWheelLocation::FR_Wheel)
    .value("BL_Wheel", cr::VehicleWheelLocation::BL_Wheel)
    .value("BR_Wheel", cr::VehicleWheelLocation::BR_Wheel)
    .value("Front_Wheel", cr::VehicleWheelLocation::Front_Wheel)
    .value("Back_Wheel", cr::VehicleWheelLocation::Back_Wheel)
  ;

  enum_<cr::VehicleDoor>("VehicleDoor")
    .value("FL", cr::VehicleDoor::FL)
    .value("FR", cr::VehicleDoor::FR)
    .value("RL", cr::VehicleDoor::RL)
    .value("RR", cr::VehicleDoor::RR)
    .value("All", cr::VehicleDoor::All)
  ;

  enum_<cr::VehicleFailureState>("VehicleFailureState")
    .value("NONE", cr::VehicleFailureState::None)
    .value("Rollover", cr::VehicleFailureState::Rollover)
    .value("Engine", cr::VehicleFailureState::Engine)
    .value("TirePuncture", cr::VehicleFailureState::TirePuncture)
  ;

  class_<cc::Vehicle, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Vehicle>>("Vehicle",
      no_init)
      .def("apply_control", &cc::Vehicle::ApplyControl, (arg("control")))
      .def("apply_ackermann_control", &cc::Vehicle::ApplyAckermannControl, (arg("control")))
      .def("get_control", &cc::Vehicle::GetControl)
      .def("set_light_state", &cc::Vehicle::SetLightState, (arg("light_state")))
      .def("open_door", &cc::Vehicle::OpenDoor, (arg("door_idx")))
      .def("close_door", &cc::Vehicle::CloseDoor, (arg("door_idx")))
      .def("set_wheel_steer_direction", &cc::Vehicle::SetWheelSteerDirection, (arg("wheel_location")), (arg("angle_in_deg")))
      .def("get_wheel_steer_angle", &cc::Vehicle::GetWheelSteerAngle, (arg("wheel_location")))
      .def("get_light_state", CONST_CALL_WITHOUT_GIL(cc::Vehicle, GetLightState))
      .def("apply_physics_control", &cc::Vehicle::ApplyPhysicsControl, (arg("physics_control")))
      .def("get_physics_control", CONST_CALL_WITHOUT_GIL(cc::Vehicle, GetPhysicsControl))
      .def("apply_ackermann_controller_settings", &cc::Vehicle::ApplyAckermannControllerSettings, (arg("settings")))
      .def("get_ackermann_controller_settings", CONST_CALL_WITHOUT_GIL(cc::Vehicle, GetAckermannControllerSettings))
      .def("set_autopilot", CALL_WITHOUT_GIL_2(cc::Vehicle, SetAutopilot, bool, uint16_t), (arg("enabled") = true, arg("tm_port") = ctm::TM_DEFAULT_PORT))
      .def("show_debug_telemetry", &cc::Vehicle::ShowDebugTelemetry, (arg("enabled") = true))
      .def("get_speed_limit", &cc::Vehicle::GetSpeedLimit)
      .def("get_traffic_light_state", &cc::Vehicle::GetTrafficLightState)
      .def("is_at_traffic_light", &cc::Vehicle::IsAtTrafficLight)
      .def("get_traffic_light", &cc::Vehicle::GetTrafficLight)
      .def("enable_carsim", &cc::Vehicle::EnableCarSim, (arg("simfile_path") = ""))
      .def("use_carsim_road", &cc::Vehicle::UseCarSimRoad, (arg("enabled")))
      .def("enable_chrono_physics", &cc::Vehicle::EnableChronoPhysics, (arg("max_substeps")=30, arg("max_substep_delta_time")=0.002, arg("vehicle_json")="", arg("powetrain_json")="", arg("tire_json")="", arg("base_json_path")=""))
      .def("get_failure_state", &cc::Vehicle::GetFailureState)
      .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Walker, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Walker>>("Walker", no_init)
      .def("apply_control", &ApplyControl<cr::WalkerControl>, (arg("control")))
      .def("get_control", &cc::Walker::GetWalkerControl)
      .def("get_bones", &cc::Walker::GetBonesTransform)
      .def("set_bones", &cc::Walker::SetBonesTransform, (arg("bones")))
      .def("blend_pose", &cc::Walker::BlendPose, (arg("blend")))
      .def("show_pose", &cc::Walker::ShowPose)
      .def("hide_pose", &cc::Walker::HidePose)
      .def("get_pose_from_animation", &cc::Walker::GetPoseFromAnimation)
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
      .def("reset_group", &cc::TrafficLight::ResetGroup)
      .def("get_affected_lane_waypoints", CALL_RETURNING_LIST(cc::TrafficLight, GetAffectedLaneWaypoints))
      .def("get_light_boxes", &GetLightBoxes)
      .def("get_opendrive_id", &cc::TrafficLight::GetOpenDRIVEID)
      .def("get_stop_waypoints", CALL_RETURNING_LIST(cc::TrafficLight, GetStopWaypoints))
      .def(self_ns::str(self_ns::self))
  ;
}
