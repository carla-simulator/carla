// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/rpc/Command.h>
#include <carla/rpc/CommandResponse.h>

namespace command_impl {

  template <typename T>
  const T &Convert(const T &in) {
    return in;
  }

  carla::rpc::ActorId Convert(const boost::shared_ptr<carla::client::Actor> &actor) {
    return actor->GetId();
  }

  carla::rpc::ActorDescription Convert(const carla::client::ActorBlueprint &blueprint) {
    return blueprint.MakeActorDescription();
  }

  template <typename... ArgsT>
  static boost::python::object CustomInit(boost::python::object self, ArgsT... args) {
    return self.attr("__init__")(Convert(args)...);
  }

  template <typename... ArgsT>
  static boost::python::object CustomSpawnActorInit(boost::python::object self, ArgsT... args) {
    return self.attr("__init__")(carla::rpc::Command::SpawnActor{Convert(args)...});
  }

  static carla::rpc::Command::SpawnActor Then(
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

  using ActorPtr = boost::shared_ptr<cc::Actor>;

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

  class_<cr::Command::ApplyWalkerControl>("ApplyWalkerControl")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cr::WalkerControl>, (arg("actor"), arg("control")))
    .def(init<cr::ActorId, cr::WalkerControl>((arg("actor_id"), arg("control"))))
    .def_readwrite("actor_id", &cr::Command::ApplyWalkerControl::actor)
    .def_readwrite("control", &cr::Command::ApplyWalkerControl::control)
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

  class_<cr::Command::ApplyVelocity>("ApplyVelocity")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("velocity")))
    .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("velocity"))))
    .def_readwrite("actor_id", &cr::Command::ApplyVelocity::actor)
    .def_readwrite("velocity", &cr::Command::ApplyVelocity::velocity)
  ;

  class_<cr::Command::ApplyAngularVelocity>("ApplyAngularVelocity")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("angular_velocity")))
    .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("angular_velocity"))))
    .def_readwrite("actor_id", &cr::Command::ApplyAngularVelocity::actor)
    .def_readwrite("angular_velocity", &cr::Command::ApplyAngularVelocity::angular_velocity)
  ;

  class_<cr::Command::ApplyImpulse>("ApplyImpulse")
    .def("__init__", &command_impl::CustomInit<ActorPtr, cg::Vector3D>, (arg("actor"), arg("impulse")))
    .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("impulse"))))
    .def_readwrite("actor_id", &cr::Command::ApplyImpulse::actor)
    .def_readwrite("impulse", &cr::Command::ApplyImpulse::impulse)
  ;

  class_<cr::Command::SetSimulatePhysics>("SetSimulatePhysics")
    .def("__init__", &command_impl::CustomInit<ActorPtr, bool>, (arg("actor"), arg("enabled")))
    .def(init<cr::ActorId, bool>((arg("actor_id"), arg("enabled"))))
    .def_readwrite("actor_id", &cr::Command::SetSimulatePhysics::actor)
    .def_readwrite("enabled", &cr::Command::SetSimulatePhysics::enabled)
  ;

  class_<cr::Command::SetAutopilot>("SetAutopilot")
    .def("__init__", &command_impl::CustomInit<ActorPtr, bool>, (arg("actor"), arg("enabled")))
    .def(init<cr::ActorId, bool>((arg("actor_id"), arg("enabled"))))
    .def_readwrite("actor_id", &cr::Command::SetAutopilot::actor)
    .def_readwrite("enabled", &cr::Command::SetAutopilot::enabled)
  ;

  implicitly_convertible<cr::Command::SpawnActor, cr::Command>();
  implicitly_convertible<cr::Command::DestroyActor, cr::Command>();
  implicitly_convertible<cr::Command::ApplyVehicleControl, cr::Command>();
  implicitly_convertible<cr::Command::ApplyWalkerControl, cr::Command>();
  implicitly_convertible<cr::Command::ApplyTransform, cr::Command>();
  implicitly_convertible<cr::Command::ApplyWalkerState, cr::Command>();
  implicitly_convertible<cr::Command::ApplyVelocity, cr::Command>();
  implicitly_convertible<cr::Command::ApplyAngularVelocity, cr::Command>();
  implicitly_convertible<cr::Command::ApplyImpulse, cr::Command>();
  implicitly_convertible<cr::Command::SetSimulatePhysics, cr::Command>();
  implicitly_convertible<cr::Command::SetAutopilot, cr::Command>();
}
