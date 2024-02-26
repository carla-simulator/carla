// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PythonAPI.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

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
    .def("__eq__", &cc::WorldSnapshot::operator==)
    .def("__ne__", &cc::WorldSnapshot::operator!=)
    .def(self_ns::str(self_ns::self))
  ;
}
