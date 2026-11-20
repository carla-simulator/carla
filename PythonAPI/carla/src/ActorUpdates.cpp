// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PythonAPI.h>

#include <carla/PythonBuffer.h>
#include <carla/rpc/ActorUpdateBatch.h>
#include <carla/rpc/VehicleLightState.h>

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace actor_updates_impl {

  namespace cr = carla::rpc;
  namespace bp = boost::python;

  using Builder = cr::ActorUpdateBatchBuilder;

  using carla::python_buffer::ReadValues;

  static std::vector<uint32_t> ReadIds(const bp::object &ids, size_t &rows) {
    rows = 0u;
    return ReadValues<uint32_t>(ids, rows, 1u, "actor_ids");
  }

  static void AddTransforms(
      Builder &self, const bp::object &ids, const bp::object &locations, const bp::object &rotations) {
    size_t n;
    const auto id = ReadIds(ids, n);
    const auto loc = ReadValues<double>(locations, n, 3u, "locations");
    const auto rot = ReadValues<float>(rotations, n, 3u, "rotations");
    for (size_t i = 0u; i < n; ++i) {
      self.AddTransform(id[i], &loc[3u * i], &rot[3u * i]);
    }
  }

  static void AddLocations(Builder &self, const bp::object &ids, const bp::object &locations) {
    size_t n;
    const auto id = ReadIds(ids, n);
    const auto loc = ReadValues<double>(locations, n, 3u, "locations");
    for (size_t i = 0u; i < n; ++i) {
      self.AddLocation(id[i], &loc[3u * i]);
    }
  }

  static void AddTargetVelocities(Builder &self, const bp::object &ids, const bp::object &velocities) {
    size_t n;
    const auto id = ReadIds(ids, n);
    const auto vel = ReadValues<float>(velocities, n, 3u, "velocities");
    for (size_t i = 0u; i < n; ++i) {
      self.AddTargetVelocity(id[i], &vel[3u * i]);
    }
  }

  static void AddTargetAngularVelocities(Builder &self, const bp::object &ids, const bp::object &velocities) {
    size_t n;
    const auto id = ReadIds(ids, n);
    const auto vel = ReadValues<float>(velocities, n, 3u, "angular_velocities");
    for (size_t i = 0u; i < n; ++i) {
      self.AddTargetAngularVelocity(id[i], &vel[3u * i]);
    }
  }

  static void AddVehicleControls(
      Builder &self, const bp::object &ids,
      const bp::object &throttle, const bp::object &steer, const bp::object &brake) {
    size_t n;
    const auto id = ReadIds(ids, n);
    const auto t = ReadValues<float>(throttle, n, 1u, "throttle");
    const auto s = ReadValues<float>(steer, n, 1u, "steer");
    const auto b = ReadValues<float>(brake, n, 1u, "brake");
    for (size_t i = 0u; i < n; ++i) {
      self.AddVehicleControl(id[i], t[i], s[i], b[i], false, false, false, 0);
    }
  }

  static void AddTransform(Builder &self, uint32_t id, const carla::geom::Transform &transform) {
    const double location[3] = {transform.location.x, transform.location.y, transform.location.z};
    const float rotation[3] = {transform.rotation.pitch, transform.rotation.yaw, transform.rotation.roll};
    self.AddTransform(id, location, rotation);
  }

  static void AddLocation(Builder &self, uint32_t id, const carla::geom::Location &location) {
    const double values[3] = {location.x, location.y, location.z};
    self.AddLocation(id, values);
  }

  static void AddWalkerState(Builder &self, uint32_t id, const carla::geom::Transform &transform, float speed) {
    const double location[3] = {transform.location.x, transform.location.y, transform.location.z};
    const float rotation[3] = {transform.rotation.pitch, transform.rotation.yaw, transform.rotation.roll};
    self.AddWalkerState(id, location, rotation, speed);
  }

  static void AddTargetVelocity(Builder &self, uint32_t id, const carla::geom::Vector3D &velocity) {
    const float values[3] = {velocity.x, velocity.y, velocity.z};
    self.AddTargetVelocity(id, values);
  }

  static void AddTargetAngularVelocity(Builder &self, uint32_t id, const carla::geom::Vector3D &velocity) {
    const float values[3] = {velocity.x, velocity.y, velocity.z};
    self.AddTargetAngularVelocity(id, values);
  }

  static void AddVehicleControl(Builder &self, uint32_t id, const cr::VehicleControl &control) {
    self.AddVehicleControl(
        id, control.throttle, control.steer, control.brake,
        control.hand_brake, control.reverse, control.manual_gear_shift, control.gear);
  }

  static void AddAckermannControl(Builder &self, uint32_t id, const cr::VehicleAckermannControl &control) {
    self.AddAckermannControl(
        id, control.steer, control.steer_speed, control.speed, control.acceleration, control.jerk);
  }

  static void AddWalkerControl(Builder &self, uint32_t id, const cr::WalkerControl &control) {
    const float direction[3] = {control.direction.x, control.direction.y, control.direction.z};
    self.AddWalkerControl(id, direction, control.speed, control.jump);
  }

  static void AddVehicleLightState(Builder &self, uint32_t id, const cr::VehicleLightState::LightState state) {
    self.AddVehicleLightState(id, static_cast<uint32_t>(state));
  }

  static void AddTrafficLightState(Builder &self, uint32_t id, const cr::TrafficLightState state) {
    self.AddTrafficLightState(id, static_cast<uint8_t>(state));
  }

  static bp::object StatusBytes(const std::vector<uint8_t> &status) {
    return bp::object(bp::handle<>(PyBytes_FromStringAndSize(
        reinterpret_cast<const char *>(status.data()), static_cast<Py_ssize_t>(status.size()))));
  }

} // namespace actor_updates_impl

/// carla.Client.apply_actor_updates
boost::python::object ApplyActorUpdates(
    const carla::client::Client &self,
    carla::rpc::ActorUpdateBatchBuilder &batch,
    bool do_tick) {
  std::vector<uint8_t> blob = batch.Finish();
  std::vector<uint8_t> status;
  {
    carla::PythonUtil::ReleaseGIL unlock;
    status = self.ApplyActorUpdates(std::move(blob), do_tick);
  }
  return actor_updates_impl::StatusBytes(status);
}

/// carla.Client.apply_actor_updates_async
void ApplyActorUpdatesAsync(
    const carla::client::Client &self,
    carla::rpc::ActorUpdateBatchBuilder &batch) {
  std::vector<uint8_t> blob = batch.Finish();
  carla::PythonUtil::ReleaseGIL unlock;
  self.ApplyActorUpdatesAsync(std::move(blob));
}

void export_actor_updates() {
  using namespace boost::python;
  using namespace actor_updates_impl;

  enum_<cr::ActorUpdateStatus>("ActorUpdateStatus")
    .value("Success", cr::ActorUpdateStatus::Success)
    .value("ActorNotFound", cr::ActorUpdateStatus::ActorNotFound)
    .value("Failed", cr::ActorUpdateStatus::Failed)
  ;

  class_<Builder, boost::noncopyable>("ActorUpdateBatch")
    .def("__len__", &Builder::size)
    .def("clear", &Builder::Clear)
    .def("add_transforms", &AddTransforms, (arg("actor_ids"), arg("locations"), arg("rotations")))
    .def("add_locations", &AddLocations, (arg("actor_ids"), arg("locations")))
    .def("add_target_velocities", &AddTargetVelocities, (arg("actor_ids"), arg("velocities")))
    .def("add_target_angular_velocities", &AddTargetAngularVelocities, (arg("actor_ids"), arg("angular_velocities")))
    .def("add_vehicle_controls", &AddVehicleControls, (arg("actor_ids"), arg("throttle"), arg("steer"), arg("brake")))
    .def("add_transform", &AddTransform, (arg("actor_id"), arg("transform")))
    .def("add_location", &AddLocation, (arg("actor_id"), arg("location")))
    .def("add_walker_state", &AddWalkerState, (arg("actor_id"), arg("transform"), arg("speed")))
    .def("add_target_velocity", &AddTargetVelocity, (arg("actor_id"), arg("velocity")))
    .def("add_target_angular_velocity", &AddTargetAngularVelocity, (arg("actor_id"), arg("angular_velocity")))
    .def("add_simulate_physics", &Builder::AddSimulatePhysics, (arg("actor_id"), arg("enabled")))
    .def("add_vehicle_control", &AddVehicleControl, (arg("actor_id"), arg("control")))
    .def("add_ackermann_control", &AddAckermannControl, (arg("actor_id"), arg("control")))
    .def("add_walker_control", &AddWalkerControl, (arg("actor_id"), arg("control")))
    .def("add_vehicle_light_state", &AddVehicleLightState, (arg("actor_id"), arg("light_state")))
    .def("add_traffic_light_state", &AddTrafficLightState, (arg("actor_id"), arg("state")))
  ;
}
