// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PythonAPI.h>

#include <carla/PythonBuffer.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <cmath>
#include <limits>
#include <optional>
#include <vector>

namespace carla {
namespace client {

  std::ostream &operator<<(std::ostream &out, const ActorSnapshot &snapshot) {
    out << "ActorSnapshot(id=" << std::to_string(snapshot.id) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const WorldSnapshot &snapshot) {
    out << "WorldSnapshot(frame=" << std::to_string(snapshot.GetTimestamp().frame) << ')';
    return out;
  }

} // namespace client
} // namespace carla

namespace snapshot_bulk {

  namespace bp = boost::python;
  namespace cc = carla::client;
  using State = cc::detail::EpisodeState;

  /// Rows of the state to read: every actor in ascending id order when @a ids
  /// is None, else the actors of @a ids in that order (no row for a missing
  /// one).
  static std::vector<std::optional<size_t>> Rows(const State &state, const bp::object &ids) {
    std::vector<std::optional<size_t>> rows;
    if (ids.is_none()) {
      rows.reserve(state.size());
      for (size_t i = 0u; i < state.size(); ++i) {
        rows.emplace_back(i);
      }
      return rows;
    }
    size_t count = 0u;
    const auto values = carla::python_buffer::ReadValues<uint32_t>(ids, count, 1u, "actor_ids");
    rows.reserve(count);
    for (const auto id : values) {
      rows.push_back(state.FindIndex(id));
    }
    return rows;
  }

  /// Fills an (N, columns) float64 array, @a out or a new one, with
  /// fill(state, row, values) for every row and NaN for missing actors.
  template <size_t Columns, typename FillT>
  static bp::object Read(
      const cc::WorldSnapshot &self,
      const bp::object &ids,
      const bp::object &out,
      bool return_mask,
      FillT fill) {
    const State &state = self.GetEpisodeState();
    const auto rows = Rows(state, ids);
    const size_t n = rows.size();
    bp::object numpy = bp::import("numpy");
    bp::object array = out.is_none() ?
        numpy.attr("empty")(bp::make_tuple(n, Columns), bp::object(numpy.attr("float64"))) :
        out;
    bp::object mask = return_mask ?
        numpy.attr("zeros")(n, bp::object(numpy.attr("bool_"))) :
        bp::object();
    {
      carla::python_buffer::WritableDoubles values(array, n * Columns, "out");
      std::optional<carla::python_buffer::WritableBools> present;
      if (return_mask) {
        present.emplace(mask, n, "mask");
      }
      carla::PythonUtil::ReleaseGIL unlock;
      constexpr double nan = std::numeric_limits<double>::quiet_NaN();
      for (size_t i = 0u; i < n; ++i) {
        double *row = values.data() + i * Columns;
        if (rows[i].has_value()) {
          fill(state, *rows[i], row);
        } else {
          for (size_t c = 0u; c < Columns; ++c) {
            row[c] = nan;
          }
        }
        if (present.has_value()) {
          present->data()[i] = rows[i].has_value() ? 1u : 0u;
        }
      }
    }
    if (return_mask) {
      return bp::make_tuple(array, mask);
    }
    return array;
  }

  static void FillVector(const carla::geom::Vector3D &v, double *row) {
    row[0u] = v.x;
    row[1u] = v.y;
    row[2u] = v.z;
  }

  static bp::object GetActorIds(const cc::WorldSnapshot &self) {
    const State &state = self.GetEpisodeState();
    bp::object numpy = bp::import("numpy");
    bp::object array = numpy.attr("empty")(state.size(), bp::object(numpy.attr("uint32")));
    carla::python_buffer::WritableUInt32 ids(array, state.size(), "actor_ids");
    size_t i = 0u;
    for (const auto id : state.GetActorIds()) {
      ids.data()[i++] = id;
    }
    return array;
  }

  static bp::object GetTransforms(
      const cc::WorldSnapshot &self, const bp::object &ids, const bp::object &out, bool return_mask) {
    return Read<6u>(self, ids, out, return_mask, [](const State &state, size_t index, double *row) {
      const auto &location = state.GetLocationAt(index);
      const auto &rotation = state.GetActorSnapshotAt(index).transform.rotation;
      row[0u] = location[0u];
      row[1u] = location[1u];
      row[2u] = location[2u];
      row[3u] = rotation.pitch;
      row[4u] = rotation.yaw;
      row[5u] = rotation.roll;
    });
  }

  static bp::object GetVelocities(
      const cc::WorldSnapshot &self, const bp::object &ids, const bp::object &out, bool return_mask) {
    return Read<3u>(self, ids, out, return_mask, [](const State &state, size_t index, double *row) {
      FillVector(state.GetActorSnapshotAt(index).velocity, row);
    });
  }

  static bp::object GetAngularVelocities(
      const cc::WorldSnapshot &self, const bp::object &ids, const bp::object &out, bool return_mask) {
    return Read<3u>(self, ids, out, return_mask, [](const State &state, size_t index, double *row) {
      FillVector(state.GetActorSnapshotAt(index).angular_velocity, row);
    });
  }

  static bp::object GetAccelerations(
      const cc::WorldSnapshot &self, const bp::object &ids, const bp::object &out, bool return_mask) {
    return Read<3u>(self, ids, out, return_mask, [](const State &state, size_t index, double *row) {
      FillVector(state.GetActorSnapshotAt(index).acceleration, row);
    });
  }

} // namespace snapshot_bulk

void export_snapshot() {
  using namespace boost::python;
  namespace cc = carla::client;

  class_<cc::ActorSnapshot>("ActorSnapshot", no_init)
    .def_readonly("id", &cc::ActorSnapshot::id)
    .def("get_transform", +[](const cc::ActorSnapshot &self) { return self.transform; })
    .def("get_velocity", +[](const cc::ActorSnapshot &self) { return self.velocity; })
    .def("get_angular_velocity", +[](const cc::ActorSnapshot &self) { return self.angular_velocity; })
    .def("get_acceleration", +[](const cc::ActorSnapshot &self) { return self.acceleration; })
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::WorldSnapshot>("WorldSnapshot", no_init)
    .add_property("id", &cc::WorldSnapshot::GetId)
    .add_property("frame", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().frame; })
    .add_property("timestamp", CALL_RETURNING_COPY(cc::WorldSnapshot, GetTimestamp))
    /// Deprecated, use timestamp @{
    .add_property("frame_count", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().frame; })
    .add_property("elapsed_seconds", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().elapsed_seconds; })
    .add_property("delta_seconds", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().delta_seconds; })
    .add_property("platform_timestamp", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().platform_timestamp; })
    /// @}
    .def("has_actor", &cc::WorldSnapshot::Contains, (arg("actor_id")))
    .def("find", CALL_RETURNING_OPTIONAL_1(cc::WorldSnapshot, Find, carla::ActorId), (arg("actor_id")))
    .def("__len__", &cc::WorldSnapshot::size)
    .def("__iter__", range(&cc::WorldSnapshot::begin, &cc::WorldSnapshot::end))
    .def("get_actor_ids", &snapshot_bulk::GetActorIds)
    .def("get_transforms", &snapshot_bulk::GetTransforms,
        (arg("actor_ids")=object(), arg("out")=object(), arg("return_mask")=false))
    .def("get_velocities", &snapshot_bulk::GetVelocities,
        (arg("actor_ids")=object(), arg("out")=object(), arg("return_mask")=false))
    .def("get_angular_velocities", &snapshot_bulk::GetAngularVelocities,
        (arg("actor_ids")=object(), arg("out")=object(), arg("return_mask")=false))
    .def("get_accelerations", &snapshot_bulk::GetAccelerations,
        (arg("actor_ids")=object(), arg("out")=object(), arg("return_mask")=false))
    .def("__eq__", &cc::WorldSnapshot::operator==)
    .def("__ne__", &cc::WorldSnapshot::operator!=)
    .def(self_ns::str(self_ns::self))
  ;
}
