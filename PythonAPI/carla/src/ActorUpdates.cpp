// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PythonAPI.h>

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

  /// Reads @a object, a C-contiguous buffer (numpy array, array.array,
  /// bytes...) of any integer or floating-point type or a sequence of
  /// numbers or of sequences of numbers, as @a rows rows of @a columns values
  /// of type T. @a rows of 0 takes the rows from the data.
  template <typename T>
  std::vector<T> ReadValues(const bp::object &object, size_t &rows, size_t columns, const char *name) {
    std::vector<T> values;
    PyObject *ptr = object.ptr();
    if (PyObject_CheckBuffer(ptr)) {
      Py_buffer view;
      if (PyObject_GetBuffer(ptr, &view, PyBUF_C_CONTIGUOUS | PyBUF_FORMAT) != 0) {
        bp::throw_error_already_set();
      }
      struct Release {
        Py_buffer *view;
        ~Release() { PyBuffer_Release(view); }
      } release{&view};
      const char *format = view.format != nullptr ? view.format : "B";
      if ((*format == '<') || (*format == '=') || (*format == '@')) {
        ++format;
      }
      const size_t count = static_cast<size_t>(view.len / view.itemsize);
      values.resize(count);
      auto copy = [&](auto tag) {
        using S = decltype(tag);
        if (static_cast<size_t>(view.itemsize) != sizeof(S)) {
          throw std::invalid_argument(std::string(name) + ": unexpected item size");
        }
        const auto *src = static_cast<const uint8_t *>(view.buf);
        for (size_t i = 0u; i < count; ++i) {
          S value;
          std::memcpy(&value, src + i * sizeof(S), sizeof(S));
          values[i] = static_cast<T>(value);
        }
      };
      switch (format[0]) {
        case 'd': copy(double{}); break;
        case 'f': copy(float{}); break;
        case 'b': copy(int8_t{}); break;
        case 'B': copy(uint8_t{}); break;
        case '?': copy(uint8_t{}); break;
        case 'h': copy(int16_t{}); break;
        case 'H': copy(uint16_t{}); break;
        case 'i': copy(int32_t{}); break;
        case 'I': copy(uint32_t{}); break;
        case 'l': view.itemsize == 8 ? copy(int64_t{}) : copy(int32_t{}); break;
        case 'L': view.itemsize == 8 ? copy(uint64_t{}) : copy(uint32_t{}); break;
        case 'q': copy(int64_t{}); break;
        case 'Q': copy(uint64_t{}); break;
        default:
          throw std::invalid_argument(
              std::string(name) + ": unsupported buffer format '" + format + "'");
      }
    } else {
      // A Python sequence of numbers, or of rows of numbers.
      const auto length = bp::len(object);
      for (decltype(bp::len(object)) i = 0; i < length; ++i) {
        bp::object item = object[i];
        if ((columns > 1u) || PySequence_Check(item.ptr())) {
          const auto width = bp::len(item);
          for (decltype(bp::len(item)) j = 0; j < width; ++j) {
            values.push_back(bp::extract<T>(item[j]));
          }
        } else {
          values.push_back(bp::extract<T>(item));
        }
      }
    }
    if (rows == 0u) {
      if (values.size() % columns != 0u) {
        throw std::invalid_argument(
            std::string(name) + ": expected " + std::to_string(columns) + " values per actor");
      }
      rows = values.size() / columns;
    } else if (values.size() != rows * columns) {
      throw std::invalid_argument(
          std::string(name) + ": expected " + std::to_string(rows) + " x " +
          std::to_string(columns) + " values, got " + std::to_string(values.size()));
    }
    return values;
  }

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
