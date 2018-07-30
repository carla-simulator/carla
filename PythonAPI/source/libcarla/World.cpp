// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Actor.h>
#include <carla/client/World.h>

#include <boost/python.hpp>

void export_world() {
  using namespace boost::python;
  namespace cc = carla::client;

  class_<cc::World, boost::noncopyable, boost::shared_ptr<cc::World>>("World", no_init)
    .def("get_blueprint_library", &cc::World::GetBlueprintLibrary)
    .def("get_spectator", &cc::World::GetSpectator)
    .def("try_spawn_actor", &cc::World::TrySpawnActor,
        (arg("blueprint"), arg("transform"), arg("attach_to")=cc::SharedPtr<cc::Actor>()))
    .def("spawn_actor", &cc::World::SpawnActor,
        (arg("blueprint"), arg("transform"), arg("attach_to")=cc::SharedPtr<cc::Actor>()))
  ;
}
