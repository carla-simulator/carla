// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PythonAPI.h>

#define TM_DEFAULT_PORT     8000

namespace command_impl {

  template <typename T>
  const T &Convert(const T &in) {
    return in;
  }

  static auto Convert(const std::shared_ptr<carla::client::Actor> &actor) {
    return actor->GetId();
  }

  static auto Convert(const carla::client::ActorBlueprint &blueprint) {
    return blueprint.MakeActorDescription();
  }

  template <typename... ArgsT>
  static auto CustomInit(boost::python::object self, ArgsT... args) {
    return self.attr("__init__")(Convert(args)...);
  }

  template <typename... ArgsT>
  static auto CustomSpawnActorInit(boost::python::object self, ArgsT... args) {
    return self.attr("__init__")(carla::rpc::Command::SpawnActor{Convert(args)...});
  }

  static auto Then(
      carla::rpc::Command::SpawnActor &self,
      carla::rpc::Command command) {
    self.do_after.push_back(command);
    return self;
  }

} // namespace command_impl

void export_commands() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cg = carla::geom;
  namespace cr = carla::rpc;

  using ActorPtr = carla::SharedPtr<cc::Actor>;

  object command_module(handle<>(borrowed(PyImport_AddModule("libcarla.command"))));
  scope().attr("command") = command_module;
  scope submodule_scope = command_module;

  // This is a handler for passing to "SpawnActor.then" commands.
  submodule_scope.attr("FutureActor") = 0u;

  class_<cr::CommandResponse>("Response", no_init)
    .add_property("actor_id", +[](const cr::CommandResponse &self) {
      return self.HasError() ? 0u : self.Get();
    })
    .add_property("error", +[](const cr::CommandResponse &self) {
      return self.HasError() ? self.GetError().What() : std::string("");
    })
    .def("has_error", &cr::CommandResponse::HasError)
  ;

  class_<cr::Command::SpawnActor>("SpawnActor")
    .def(
        "__init__",
        &command_impl::CustomSpawnActorInit<cc::ActorBlueprint, cg::Transform>,
        (arg("blueprint"), arg("transform")))
    .def(
        "__init__",
        &command_impl::CustomSpawnActorInit<cc::ActorBlueprint, cg::Transform, const cr::ActorId &>,
        (arg("blueprint"), arg("transform"), arg("parent_id")))
    .def(
        "__init__",
        &command_impl::CustomSpawnActorInit<cc::ActorBlueprint, cg::Transform, ActorPtr>,
        (arg("blueprint"), arg("transform"), arg("parent")))
    .def(init<cr::Command::SpawnActor>())
    .def_readwrite("transform", &cr::Command::SpawnActor::transform)
    .def_readwrite("parent_id", &cr::Command::SpawnActor::parent)
    .def("then", &command_impl::Then, (arg("command")))
  ;

  class_<cr::Command::DestroyActor>("DestroyActor")
    .def("__init__", &command_impl::CustomInit<ActorPtr>, (arg("actor")))
    .def(init<cr::ActorId>((arg("actor_id"))))
    .def_readwrite("actor_id", &cr::Command::DestroyActor::actor)
  ;

  class_<cr::Command::ApplyVehicleControl>("ApplyVehicleControl")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cr::VehicleControl>, (arg("actor"), arg("control")))
    .def(init<cr::ActorId, cr::VehicleControl>((arg("actor_id"), arg("control"))))
    .def_readwrite("actor_id", &cr::Command::ApplyVehicleControl::actor)
    .def_readwrite("control", &cr::Command::ApplyVehicleControl::control)
  ;

  class_<cr::Command::ApplyVehicleAckermannControl>("ApplyVehicleAckermannControl")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cr::VehicleAckermannControl>, (arg("actor"), arg("control")))
    .def(init<cr::ActorId, cr::VehicleAckermannControl>((arg("actor_id"), arg("control"))))
    .def_readwrite("actor_id", &cr::Command::ApplyVehicleAckermannControl::actor)
    .def_readwrite("control", &cr::Command::ApplyVehicleAckermannControl::control)
  ;

  class_<cr::Command::ApplyWalkerControl>("ApplyWalkerControl")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cr::WalkerControl>, (arg("actor"), arg("control")))
    .def(init<cr::ActorId, cr::WalkerControl>((arg("actor_id"), arg("control"))))
    .def_readwrite("actor_id", &cr::Command::ApplyWalkerControl::actor)
    .def_readwrite("control", &cr::Command::ApplyWalkerControl::control)
  ;

  class_<cr::Command::ApplyVehiclePhysicsControl>("ApplyVehiclePhysicsControl")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cr::VehiclePhysicsControl>, (arg("actor"), arg("physics_control")))
    .def(init<cr::ActorId, cr::VehiclePhysicsControl>((arg("actor_id"), arg("physics_control"))))
    .def_readwrite("actor_id", &cr::Command::ApplyVehiclePhysicsControl::actor)
    .def_readwrite("physics_control", &cr::Command::ApplyVehiclePhysicsControl::physics_control)
  ;

  class_<cr::Command::ApplyTransform>("ApplyTransform")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Transform>, (arg("actor"), arg("transform")))
    .def(init<cr::ActorId, cg::Transform>((arg("actor_id"), arg("transform"))))
    .def_readwrite("actor_id", &cr::Command::ApplyTransform::actor)
    .def_readwrite("transform", &cr::Command::ApplyTransform::transform)
  ;

  class_<cr::Command::ApplyWalkerState>("ApplyWalkerState")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Transform, float>, (arg("actor"), arg("transform"), arg("speed")))
    .def(init<cr::ActorId, cg::Transform, float>((arg("actor_id"), arg("transform"), arg("speed"))))
    .def_readwrite("actor_id", &cr::Command::ApplyWalkerState::actor)
    .def_readwrite("transform", &cr::Command::ApplyWalkerState::transform)
    .def_readwrite("speed", &cr::Command::ApplyWalkerState::speed)
  ;

  class_<cr::Command::ApplyTargetVelocity>("ApplyTargetVelocity")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("velocity")))
    .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("velocity"))))
    .def_readwrite("actor_id", &cr::Command::ApplyTargetVelocity::actor)
    .def_readwrite("velocity", &cr::Command::ApplyTargetVelocity::velocity)
  ;

  class_<cr::Command::ApplyTargetAngularVelocity>("ApplyTargetAngularVelocity")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("angular_velocity")))
    .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("angular_velocity"))))
    .def_readwrite("actor_id", &cr::Command::ApplyTargetAngularVelocity::actor)
    .def_readwrite("angular_velocity", &cr::Command::ApplyTargetAngularVelocity::angular_velocity)
  ;

  class_<cr::Command::ApplyImpulse>("ApplyImpulse")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("impulse")))
    .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("impulse"))))
    .def_readwrite("actor_id", &cr::Command::ApplyImpulse::actor)
    .def_readwrite("impulse", &cr::Command::ApplyImpulse::impulse)
  ;

  class_<cr::Command::ApplyForce>("ApplyForce")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("force")))
    .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("force"))))
    .def_readwrite("actor_id", &cr::Command::ApplyForce::actor)
    .def_readwrite("force", &cr::Command::ApplyForce::force)
  ;

  class_<cr::Command::ApplyAngularImpulse>("ApplyAngularImpulse")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("impulse")))
    .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("impulse"))))
    .def_readwrite("actor_id", &cr::Command::ApplyAngularImpulse::actor)
    .def_readwrite("impulse", &cr::Command::ApplyAngularImpulse::impulse)
  ;

  class_<cr::Command::ApplyTorque>("ApplyTorque")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("torque")))
    .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("torque"))))
    .def_readwrite("actor_id", &cr::Command::ApplyTorque::actor)
    .def_readwrite("torque", &cr::Command::ApplyTorque::torque)
  ;

  class_<cr::Command::SetSimulatePhysics>("SetSimulatePhysics")
    .def("__init__", &command_impl::CustomInit<ActorPtr, bool>, (arg("actor"), arg("enabled")))
    .def(init<cr::ActorId, bool>((arg("actor_id"), arg("enabled"))))
    .def_readwrite("actor_id", &cr::Command::SetSimulatePhysics::actor)
    .def_readwrite("enabled", &cr::Command::SetSimulatePhysics::enabled)
  ;

  class_<cr::Command::SetEnableGravity>("SetEnableGravity")
    .def("__init__", &command_impl::CustomInit<ActorPtr, bool>, (arg("actor"), arg("enabled")))
    .def(init<cr::ActorId, bool>((arg("actor_id"), arg("enabled"))))
    .def_readwrite("actor_id", &cr::Command::SetEnableGravity::actor)
    .def_readwrite("enabled", &cr::Command::SetEnableGravity::enabled)
  ;

  class_<cr::Command::SetAutopilot>("SetAutopilot")
    .def("__init__", &command_impl::CustomInit<ActorPtr, bool, uint16_t>, (arg("actor"), arg("enabled"), arg("tm_port") = TM_DEFAULT_PORT ))
    .def(init<cr::ActorId, bool, uint16_t>((arg("actor_id"), arg("enabled"), arg("tm_port") = TM_DEFAULT_PORT )))
    .def_readwrite("actor_id", &cr::Command::SetAutopilot::actor)
    .def_readwrite("tm_port", &cr::Command::SetAutopilot::tm_port)
    .def_readwrite("enabled", &cr::Command::SetAutopilot::enabled)
  ;

  class_<cr::Command::ShowDebugTelemetry>("ShowDebugTelemetry")
    .def("__init__", &command_impl::CustomInit<ActorPtr, bool, uint16_t>, (arg("actor"), arg("enabled")))
    .def(init<cr::ActorId, bool>((arg("actor_id"), arg("enabled"))))
    .def_readwrite("actor_id", &cr::Command::ShowDebugTelemetry::actor)
    .def_readwrite("enabled", &cr::Command::ShowDebugTelemetry::enabled)
  ;

  class_<cr::Command::SetVehicleLightState>("SetVehicleLightState")
    .def("__init__", &command_impl::CustomInit<ActorPtr, bool>, (arg("actor"), arg("light_state")))
    .def(init<cr::ActorId, cr::VehicleLightState::flag_type>((arg("actor_id"), arg("light_state"))))
    .def_readwrite("actor_id", &cr::Command::SetVehicleLightState::actor)
    .def_readwrite("light_state", &cr::Command::SetVehicleLightState::light_state)
  ;

  implicitly_convertible<cr::Command::SpawnActor, cr::Command>();
  implicitly_convertible<cr::Command::DestroyActor, cr::Command>();
  implicitly_convertible<cr::Command::ApplyVehicleControl, cr::Command>();
  implicitly_convertible<cr::Command::ApplyVehicleAckermannControl, cr::Command>();
  implicitly_convertible<cr::Command::ApplyWalkerControl, cr::Command>();
  implicitly_convertible<cr::Command::ApplyVehiclePhysicsControl, cr::Command>();
  implicitly_convertible<cr::Command::ApplyTransform, cr::Command>();
  implicitly_convertible<cr::Command::ApplyWalkerState, cr::Command>();
  implicitly_convertible<cr::Command::ApplyTargetVelocity, cr::Command>();
  implicitly_convertible<cr::Command::ApplyTargetAngularVelocity, cr::Command>();
  implicitly_convertible<cr::Command::ApplyImpulse, cr::Command>();
  implicitly_convertible<cr::Command::ApplyForce, cr::Command>();
  implicitly_convertible<cr::Command::ApplyAngularImpulse, cr::Command>();
  implicitly_convertible<cr::Command::ApplyTorque, cr::Command>();
  implicitly_convertible<cr::Command::SetSimulatePhysics, cr::Command>();
  implicitly_convertible<cr::Command::SetEnableGravity, cr::Command>();
  implicitly_convertible<cr::Command::SetAutopilot, cr::Command>();
  implicitly_convertible<cr::Command::SetVehicleLightState, cr::Command>();
}
