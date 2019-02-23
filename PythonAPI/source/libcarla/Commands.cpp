// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/rpc/Command.h>

void export_commands() {
  using namespace boost::python;
  namespace cr = carla::rpc;
  namespace cg = carla::geom;

  object command_module(handle<>(borrowed(PyImport_AddModule("libcarla.command"))));
  scope().attr("command") = command_module;
  scope io_scope = command_module;

  class_<cr::Command::DestroyActor>("DestroyActor")
    .def(init<cr::ActorId>((arg("actor_id"))))
    .def_readwrite("actor_id", &cr::Command::DestroyActor::actor)
  ;

  class_<cr::Command::ApplyVehicleControl>("ApplyVehicleControl")
    .def(init<cr::ActorId, cr::VehicleControl>((arg("actor_id"), arg("control"))))
    .def_readwrite("actor_id", &cr::Command::ApplyVehicleControl::actor)
    .def_readwrite("control", &cr::Command::ApplyVehicleControl::control)
  ;

  class_<cr::Command::ApplyWalkerControl>("ApplyWalkerControl")
    .def(init<cr::ActorId, cr::WalkerControl>((arg("actor_id"), arg("control"))))
    .def_readwrite("actor_id", &cr::Command::ApplyWalkerControl::actor)
    .def_readwrite("control", &cr::Command::ApplyWalkerControl::control)
  ;

  class_<cr::Command::ApplyTransform>("ApplyTransform")
    .def(init<cr::ActorId, cg::Transform>((arg("actor_id"), arg("transform"))))
    .def_readwrite("actor_id", &cr::Command::ApplyTransform::actor)
    .def_readwrite("transform", &cr::Command::ApplyTransform::transform)
  ;

  class_<cr::Command::ApplyVelocity>("ApplyVelocity")
    .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("velocity"))))
    .def_readwrite("actor_id", &cr::Command::ApplyVelocity::actor)
    .def_readwrite("velocity", &cr::Command::ApplyVelocity::velocity)
  ;

  class_<cr::Command::ApplyAngularVelocity>("ApplyAngularVelocity")
    .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("angular_velocity"))))
    .def_readwrite("actor_id", &cr::Command::ApplyAngularVelocity::actor)
    .def_readwrite("angular_velocity", &cr::Command::ApplyAngularVelocity::angular_velocity)
  ;

  class_<cr::Command::ApplyImpulse>("ApplyImpulse")
    .def(init<cr::ActorId, cg::Vector3D>((arg("actor_id"), arg("impulse"))))
    .def_readwrite("actor_id", &cr::Command::ApplyImpulse::actor)
    .def_readwrite("impulse", &cr::Command::ApplyImpulse::impulse)
  ;

  class_<cr::Command::SetSimulatePhysics>("SetSimulatePhysics")
    .def(init<cr::ActorId, bool>((arg("actor_id"), arg("enabled"))))
    .def_readwrite("actor_id", &cr::Command::SetSimulatePhysics::actor)
    .def_readwrite("enabled", &cr::Command::SetSimulatePhysics::enabled)
  ;

  class_<cr::Command::SetAutopilot>("SetAutopilot")
    .def(init<cr::ActorId, bool>((arg("actor_id"), arg("enabled"))))
    .def_readwrite("actor_id", &cr::Command::SetAutopilot::actor)
    .def_readwrite("enabled", &cr::Command::SetAutopilot::enabled)
  ;

  implicitly_convertible<cr::Command::DestroyActor, cr::Command>();
  implicitly_convertible<cr::Command::ApplyVehicleControl, cr::Command>();
  implicitly_convertible<cr::Command::ApplyWalkerControl, cr::Command>();
  implicitly_convertible<cr::Command::ApplyTransform, cr::Command>();
  implicitly_convertible<cr::Command::ApplyVelocity, cr::Command>();
  implicitly_convertible<cr::Command::ApplyAngularVelocity, cr::Command>();
  implicitly_convertible<cr::Command::ApplyImpulse, cr::Command>();
  implicitly_convertible<cr::Command::SetSimulatePhysics, cr::Command>();
  implicitly_convertible<cr::Command::SetAutopilot, cr::Command>();
}
