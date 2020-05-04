// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/client/Actor.h>
#include <carla/client/ActorList.h>
#include <carla/client/World.h>

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
    .def_readonly("id", &cc::ActorSnapshot::id, "@DocString(ActorSnapshot.id)")
    .def("get_transform", +[](const cc::ActorSnapshot &self) { return self.transform; }, "@DocString(ActorSnapshot.get_transform)")
    .def("get_velocity", +[](const cc::ActorSnapshot &self) { return self.velocity; }, "@DocString(ActorSnapshot.get_velocity)")
    .def("get_angular_velocity", +[](const cc::ActorSnapshot &self) { return self.angular_velocity; }, "@DocString(ActorSnapshot.get_angular_velocity)")
    .def("get_acceleration", +[](const cc::ActorSnapshot &self) { return self.acceleration; }, "@DocString(ActorSnapshot.get_acceleration)")
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::WorldSnapshot>("WorldSnapshot", no_init)
    .add_property("id", &cc::WorldSnapshot::GetId, "@DocString(WorldSnapshot.id)")
    .add_property("frame", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().frame; }, "@DocString(WorldSnapshot.frame)")
    .add_property("timestamp", CALL_RETURNING_COPY(cc::WorldSnapshot, GetTimestamp), "@DocString(WorldSnapshot.timestamp)")
    /// Deprecated, use timestamp @{
    .add_property("frame_count", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().frame; }, "@DocString(WorldSnapshot.frame_count)")
    .add_property("elapsed_seconds", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().elapsed_seconds; }, "@DocString(WorldSnapshot.elapsed_seconds)")
    .add_property("delta_seconds", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().delta_seconds; }, "@DocString(WorldSnapshot.delta_seconds)")
    .add_property("platform_timestamp", +[](const cc::WorldSnapshot &self) { return self.GetTimestamp().platform_timestamp; }, "@DocString(WorldSnapshot.platform_timestamp)")
    /// @}
    .def("has_actor", &cc::WorldSnapshot::Contains, (arg("actor_id")), "@DocString(WorldSnapshot.has_actor)")
    .def("find", CALL_RETURNING_OPTIONAL_1(cc::WorldSnapshot, Find, carla::ActorId), (arg("actor_id")), "@DocString(WorldSnapshot.find)")
    .def("__len__", &cc::WorldSnapshot::size, "@DocString(WorldSnapshot.__len__)")
    .def("__iter__", range(&cc::WorldSnapshot::begin, &cc::WorldSnapshot::end), "@DocString(WorldSnapshot.__iter__)")
    .def("__eq__", &cc::WorldSnapshot::operator==, "@DocString(WorldSnapshot.__eq__)")
    .def("__ne__", &cc::WorldSnapshot::operator!=, "@DocString(WorldSnapshot.__ne__)")
    .def(self_ns::str(self_ns::self))
  ;
}
