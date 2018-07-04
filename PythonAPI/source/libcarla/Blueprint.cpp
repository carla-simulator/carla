// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/BlueprintLibrary.h>
#include <carla/client/ActorBlueprint.h>

#include <boost/python.hpp>

#include <ostream>

namespace carla {
namespace client {

  std::ostream &operator<<(std::ostream &out, const ActorBlueprint &bp) {
    out << bp.GetTypeId();
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const BlueprintLibrary &blueprints) {
    auto it = blueprints.begin();
    out << '[' << *it;
    for (++it; it != blueprints.end(); ++it) {
      out << ", " << *it;
    }
    out << ']';
    return out;
  }

} // namespace client
} // namespace carla

void export_blueprint() {
  using namespace boost::python;
  namespace cc = carla::client;

  class_<cc::ActorBlueprint>("ActorBlueprint", no_init)
    .add_property("type_id", +[](const cc::ActorBlueprint &self) -> std::string {
      return self.GetTypeId();
    })
    .def("startswith", &cc::ActorBlueprint::StartsWith)
    .def("match", &cc::ActorBlueprint::MatchWildcards)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::BlueprintLibrary>("BlueprintLibrary", no_init)
    .def("filter", &cc::BlueprintLibrary::Filter)
    .def("__getitem__", +[](const cc::BlueprintLibrary &self, size_t pos) -> cc::ActorBlueprint {
      return self[pos];
    })
    .def("__len__", &cc::BlueprintLibrary::size)
    .def("__iter__", range(&cc::BlueprintLibrary::begin, &cc::BlueprintLibrary::end))
    .def(self_ns::str(self_ns::self))
  ;
}
