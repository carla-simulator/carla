// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/client/Actor.h>
#include <carla/client/World.h>

void export_world() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cg = carla::geom;

#define SPAWN_ACTOR_WITHOUT_GIL(fn) +[]( \
        cc::World &self, \
        const cc::ActorBlueprint &blueprint, \
        const cg::Transform &transform, \
        cc::Actor *parent) { \
      carla::PythonUtil::ReleaseGIL unlock; \
      return self.fn(blueprint, transform, parent); \
    }, \
    (arg("blueprint"), arg("transform"), arg("attach_to")=carla::SharedPtr<cc::Actor>())

  class_<cc::World>("World", no_init)
    .add_property("id", &cc::World::GetId)
    .add_property("map_name", CALL_RETURNING_COPY(cc::World, GetMapName))
    .def("get_blueprint_library", CONST_CALL_WITHOUT_GIL(cc::World, GetBlueprintLibrary))
    .def("get_spectator", CONST_CALL_WITHOUT_GIL(cc::World, GetSpectator))
    .def("get_weather", CONST_CALL_WITHOUT_GIL(cc::World, GetWeather))
    .def("set_weather", &cc::World::SetWeather)
    .def("try_spawn_actor", SPAWN_ACTOR_WITHOUT_GIL(TrySpawnActor))
    .def("spawn_actor", SPAWN_ACTOR_WITHOUT_GIL(SpawnActor))
  ;

#undef SPAWN_ACTOR_WITHOUT_GIL
}
