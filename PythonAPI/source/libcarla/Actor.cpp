// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Actor.h>

#include <boost/python.hpp>

#include <ostream>

namespace carla {
namespace client {

  std::ostream &operator<<(std::ostream &out, const Actor &actor) {
    out << "Actor(id=" << actor.GetId() << ", type_id=" << actor.GetTypeId() << ')';
    return out;
  }

} // namespace client
} // namespace carla

void export_actor() {
  using namespace boost::python;
  namespace cc = carla::client;

  class_<cc::Actor, boost::noncopyable, boost::shared_ptr<cc::Actor>>("Actor", no_init)
    .add_property("id", &cc::Actor::GetId)
    .add_property("type_id", +[](const cc::Actor &self) -> std::string {
      // Needs to copy the string by value.
      return self.GetTypeId();
    })
    .add_property("blueprint", &cc::Actor::GetBlueprint)
    .def("get_world", &cc::Actor::GetWorld)
    .def("apply_control", &cc::Actor::ApplyControl)
    .def(self_ns::str(self_ns::self))
  ;
}
