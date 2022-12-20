// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/client/Actor.h>
#include <carla/client/ActorList.h>
#include <carla/client/World.h>
#include <carla/rpc/EnvironmentObject.h>
#include <carla/rpc/ObjectLabel.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

namespace carla {
namespace client {

  std::ostream &operator<<(std::ostream &out, const ActorList &actors) {
    return PrintList(out, actors);
  }

  std::ostream &operator<<(std::ostream &out, const World &world) {
    out << "World(id=" << world.GetId() << ')';
    return out;
  }

} // namespace client
} // namespace carla

namespace carla {
namespace rpc {

  std::ostream &operator<<(std::ostream &out, const EpisodeSettings &settings) {
    auto BoolToStr = [](bool b) { return b ? "True" : "False"; };
    out << "WorldSettings(synchronous_mode=" << BoolToStr(settings.synchronous_mode)
        << ",no_rendering_mode=" << BoolToStr(settings.no_rendering_mode)
        << ",fixed_delta_seconds=" << settings.fixed_delta_seconds.get()
        << ",substepping=" << BoolToStr(settings.substepping)
        << ",max_substep_delta_time=" << settings.max_substep_delta_time
        << ",max_substeps=" << settings.max_substeps
        << ",max_culling_distance=" << settings.max_culling_distance
        << ",deterministic_ragdolls=" << BoolToStr(settings.deterministic_ragdolls) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const EnvironmentObject &environment_object) {
    out << "Mesh(id=" << environment_object.id << ", ";
    out << "name=" << environment_object.name << ", ";
    out << "transform=" << environment_object.transform << ", ";
    out << "bounding_box=" << environment_object.bounding_box << ")";
    return out;
  }

} // namespace rpc
} // namespace carla

static auto WaitForTick(const carla::client::World &world, double seconds) {
  carla::PythonUtil::ReleaseGIL unlock;
  return world.WaitForTick(TimeDurationFromSeconds(seconds));
}

static size_t OnTick(carla::client::World &self, boost::python::object callback) {
  return self.OnTick(MakeCallback(std::move(callback)));
}

static auto Tick(carla::client::World &world, double seconds) {
  carla::PythonUtil::ReleaseGIL unlock;
  return world.Tick(TimeDurationFromSeconds(seconds));
}

static auto ApplySettings(carla::client::World &world, carla::rpc::EpisodeSettings settings, double seconds) {
  carla::PythonUtil::ReleaseGIL unlock;
  return world.ApplySettings(settings, TimeDurationFromSeconds(seconds));
}

static auto GetActorsById(carla::client::World &self, const boost::python::list &actor_ids) {
  std::vector<carla::ActorId> ids{
      boost::python::stl_input_iterator<carla::ActorId>(actor_ids),
      boost::python::stl_input_iterator<carla::ActorId>()};
  carla::PythonUtil::ReleaseGIL unlock;
  return self.GetActors(ids);
}

static auto GetVehiclesLightStates(carla::client::World &self) {
  boost::python::dict dict;
  auto list = self.GetVehiclesLightStates();
  for (auto &vehicle : list) {
    dict[vehicle.first] = vehicle.second;
  }
  return dict;
}

static auto GetLevelBBs(const carla::client::World &self, uint8_t queried_tag) {
  boost::python::list result;
  for (const auto &bb : self.GetLevelBBs(queried_tag)) {
    result.append(bb);
  }
  return result;
}

static auto GetEnvironmentObjects(const carla::client::World &self, uint8_t queried_tag) {
  boost::python::list result;
  for (const auto &object : self.GetEnvironmentObjects(queried_tag)) {
    result.append(object);
  }
  return result;
}

static void EnableEnvironmentObjects(
  carla::client::World &self,
  const boost::python::object& py_env_objects_ids,
  const bool enable ) {

  std::vector<uint64_t> env_objects_ids {
    boost::python::stl_input_iterator<uint64_t>(py_env_objects_ids),
    boost::python::stl_input_iterator<uint64_t>()
  };

  self.EnableEnvironmentObjects(env_objects_ids, enable);
}

void export_world() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cg = carla::geom;
  namespace cr = carla::rpc;
  namespace csd = carla::sensor::data;

  class_<cc::Timestamp>("Timestamp")
    .def(init<size_t, double, double, double>(
        (arg("frame")=0u,
         arg("elapsed_seconds")=0.0,
         arg("delta_seconds")=0.0,
         arg("platform_timestamp")=0.0)))
    .def_readwrite("frame", &cc::Timestamp::frame)
    .def_readwrite("frame_count", &cc::Timestamp::frame) // deprecated.
    .def_readwrite("elapsed_seconds", &cc::Timestamp::elapsed_seconds)
    .def_readwrite("delta_seconds", &cc::Timestamp::delta_seconds)
    .def_readwrite("platform_timestamp", &cc::Timestamp::platform_timestamp)
    .def("__eq__", &cc::Timestamp::operator==)
    .def("__ne__", &cc::Timestamp::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::ActorList, boost::shared_ptr<cc::ActorList>>("ActorList", no_init)
    .def("find", &cc::ActorList::Find, (arg("id")))
    .def("filter", &cc::ActorList::Filter, (arg("wildcard_pattern")))
    .def("__getitem__", &cc::ActorList::at)
    .def("__len__", &cc::ActorList::size)
    .def("__iter__", range(&cc::ActorList::begin, &cc::ActorList::end))
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::EpisodeSettings>("WorldSettings")
    .def(init<bool, bool, double, bool, double, int, float, bool, float, float>(
        (arg("synchronous_mode")=false,
         arg("no_rendering_mode")=false,
         arg("fixed_delta_seconds")=0.0,
         arg("substepping")=true,
         arg("max_substep_delta_time")=0.01,
         arg("max_substeps")=10,
         arg("max_culling_distance")=0.0f,
         arg("deterministic_ragdolls")=false,
         arg("tile_stream_distance")=3000.f,
         arg("actor_active_distance")=2000.f)))
    .def_readwrite("synchronous_mode", &cr::EpisodeSettings::synchronous_mode)
    .def_readwrite("no_rendering_mode", &cr::EpisodeSettings::no_rendering_mode)
    .def_readwrite("substepping", &cr::EpisodeSettings::substepping)
    .def_readwrite("max_substep_delta_time", &cr::EpisodeSettings::max_substep_delta_time)
    .def_readwrite("max_substeps", &cr::EpisodeSettings::max_substeps)
    .def_readwrite("max_culling_distance", &cr::EpisodeSettings::max_culling_distance)
    .def_readwrite("deterministic_ragdolls", &cr::EpisodeSettings::deterministic_ragdolls)
    .add_property("fixed_delta_seconds",
        +[](const cr::EpisodeSettings &self) {
          return OptionalToPythonObject(self.fixed_delta_seconds);
        },
        +[](cr::EpisodeSettings &self, object value) {
          double fds = (value == object{} ? 0.0 : extract<double>(value));
          self.fixed_delta_seconds = fds > 0.0 ? fds : boost::optional<double>{};
        })
    .def_readwrite("tile_stream_distance", &cr::EpisodeSettings::tile_stream_distance)
    .def_readwrite("actor_active_distance", &cr::EpisodeSettings::actor_active_distance)
    .def("__eq__", &cr::EpisodeSettings::operator==)
    .def("__ne__", &cr::EpisodeSettings::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::EnvironmentObject>("EnvironmentObject", no_init)
    .def_readwrite("transform", &cr::EnvironmentObject::transform)
    .def_readwrite("bounding_box", &cr::EnvironmentObject::bounding_box)
    .def_readwrite("id", &cr::EnvironmentObject::id)
    .def_readwrite("name", &cr::EnvironmentObject::name)
    .def_readwrite("type", &cr::EnvironmentObject::type)
    .def(self_ns::str(self_ns::self))
  ;

  enum_<cr::AttachmentType>("AttachmentType")
    .value("Rigid", cr::AttachmentType::Rigid)
    .value("SpringArm", cr::AttachmentType::SpringArm)
    .value("SpringArmGhost", cr::AttachmentType::SpringArmGhost)
  ;

  enum_<cr::CityObjectLabel>("CityObjectLabel")
    .value("NONE", cr::CityObjectLabel::None)
    .value("Buildings", cr::CityObjectLabel::Buildings)
    .value("Fences", cr::CityObjectLabel::Fences)
    .value("Other", cr::CityObjectLabel::Other)
    .value("Pedestrians", cr::CityObjectLabel::Pedestrians)
    .value("Poles", cr::CityObjectLabel::Poles)
    .value("RoadLines", cr::CityObjectLabel::RoadLines)
    .value("Roads", cr::CityObjectLabel::Roads)
    .value("Sidewalks", cr::CityObjectLabel::Sidewalks)
    .value("TrafficSigns", cr::CityObjectLabel::TrafficSigns)
    .value("Vegetation", cr::CityObjectLabel::Vegetation)
    .value("Car", cr::CityObjectLabel::Car)
    .value("Walls", cr::CityObjectLabel::Walls)
    .value("Sky", cr::CityObjectLabel::Sky)
    .value("Ground", cr::CityObjectLabel::Ground)
    .value("Bridge", cr::CityObjectLabel::Bridge)
    .value("RailTrack", cr::CityObjectLabel::RailTrack)
    .value("GuardRail", cr::CityObjectLabel::GuardRail)
    .value("TrafficLight", cr::CityObjectLabel::TrafficLight)
    .value("Static", cr::CityObjectLabel::Static)
    .value("Dynamic", cr::CityObjectLabel::Dynamic)
    .value("Water", cr::CityObjectLabel::Water)
    .value("Terrain", cr::CityObjectLabel::Terrain)
    .value("Truck", cr::CityObjectLabel::Truck)
    .value("Motorcycle", cr::CityObjectLabel::Motorcycle)
    .value("Bicycle", cr::CityObjectLabel::Bicycle)
    .value("Bus", cr::CityObjectLabel::Bus)
    .value("Rider", cr::CityObjectLabel::Rider)
    .value("Train", cr::CityObjectLabel::Train)
    .value("Any", cr::CityObjectLabel::Any)
  ;

  class_<cr::LabelledPoint>("LabelledPoint", no_init)
    .def_readonly("location", &cr::LabelledPoint::_location)
    .def_readonly("label", &cr::LabelledPoint::_label)
  ;

  enum_<cr::MapLayer>("MapLayer")
    .value("NONE", cr::MapLayer::None)
    .value("Buildings", cr::MapLayer::Buildings)
    .value("Decals", cr::MapLayer::Decals)
    .value("Foliage", cr::MapLayer::Foliage)
    .value("Ground", cr::MapLayer::Ground)
    .value("ParkedVehicles", cr::MapLayer::ParkedVehicles)
    .value("Particles", cr::MapLayer::Particles)
    .value("Props", cr::MapLayer::Props)
    .value("StreetLights", cr::MapLayer::StreetLights)
    .value("Walls", cr::MapLayer::Walls)
    .value("All", cr::MapLayer::All)
  ;

  enum_<cr::MaterialParameter>("MaterialParameter")
    .value("Normal", cr::MaterialParameter::Tex_Normal)
    .value("AO_Roughness_Metallic_Emissive", cr::MaterialParameter::Tex_Ao_Roughness_Metallic_Emissive)
    .value("Diffuse", cr::MaterialParameter::Tex_Diffuse)
    .value("Emissive", cr::MaterialParameter::Tex_Emissive)
  ;

  class_<cr::TextureColor>("TextureColor")
    .def(init<uint32_t, uint32_t>())
    .add_property("width", &cr::TextureColor::GetWidth)
    .add_property("height", &cr::TextureColor::GetHeight)
    .def("set_dimensions", &cr::TextureColor::SetDimensions)
    .def("get", +[](const cr::TextureColor &self, int x, int y) -> csd::Color{
      return self.At(static_cast<uint32_t>(x), static_cast<uint32_t>(y));
    })
    .def("set", +[](cr::TextureColor &self, int x, int y, csd::Color& value) {
      self.At(static_cast<uint32_t>(x), static_cast<uint32_t>(y)) = value;
    })
  ;

  class_<cr::TextureFloatColor>("TextureFloatColor")
    .def(init<uint32_t, uint32_t>())
    .add_property("width", &cr::TextureFloatColor::GetWidth)
    .add_property("height", &cr::TextureFloatColor::GetHeight)
    .def("set_dimensions", &cr::TextureFloatColor::SetDimensions)
    .def("get", +[](const cr::TextureFloatColor &self, int x, int y) -> cr::FloatColor{
      return self.At(static_cast<uint32_t>(x), static_cast<uint32_t>(y));
    })
    .def("set", +[](cr::TextureFloatColor &self, int x, int y, cr::FloatColor& value) {
      self.At(static_cast<uint32_t>(x), static_cast<uint32_t>(y)) = value;
    })
  ;

#define SPAWN_ACTOR_WITHOUT_GIL(fn) +[]( \
        cc::World &self, \
        const cc::ActorBlueprint &blueprint, \
        const cg::Transform &transform, \
        cc::Actor *parent, \
        cr::AttachmentType attachment_type) { \
      carla::PythonUtil::ReleaseGIL unlock; \
      return self.fn(blueprint, transform, parent, attachment_type); \
    }, \
    ( \
      arg("blueprint"), \
      arg("transform"), \
      arg("attach_to")=carla::SharedPtr<cc::Actor>(), \
      arg("attachment_type")=cr::AttachmentType::Rigid)

  class_<cc::World>("World", no_init)
    .add_property("id", &cc::World::GetId)
    .add_property("debug", &cc::World::MakeDebugHelper)
    .def("load_map_layer", CONST_CALL_WITHOUT_GIL_1(cc::World, LoadLevelLayer, cr::MapLayer), arg("map_layers"))
    .def("unload_map_layer", CONST_CALL_WITHOUT_GIL_1(cc::World, UnloadLevelLayer, cr::MapLayer), arg("map_layers"))
    .def("get_blueprint_library", CONST_CALL_WITHOUT_GIL(cc::World, GetBlueprintLibrary))
    .def("get_vehicles_light_states", &GetVehiclesLightStates)
    .def("get_map", CONST_CALL_WITHOUT_GIL(cc::World, GetMap))
    .def("get_random_location_from_navigation", CALL_RETURNING_OPTIONAL_WITHOUT_GIL(cc::World, GetRandomLocationFromNavigation))
    .def("get_spectator", CONST_CALL_WITHOUT_GIL(cc::World, GetSpectator))
    .def("get_settings", CONST_CALL_WITHOUT_GIL(cc::World, GetSettings))
    .def("apply_settings", &ApplySettings, (arg("settings"), arg("seconds")=0.0))
    .def("get_weather", CONST_CALL_WITHOUT_GIL(cc::World, GetWeather))
    .def("set_weather", &cc::World::SetWeather)
    .def("get_snapshot", &cc::World::GetSnapshot)
    .def("get_actor", CONST_CALL_WITHOUT_GIL_1(cc::World, GetActor, carla::ActorId), (arg("actor_id")))
    .def("get_actors", CONST_CALL_WITHOUT_GIL(cc::World, GetActors))
    .def("get_actors", &GetActorsById, (arg("actor_ids")))
    .def("spawn_actor", SPAWN_ACTOR_WITHOUT_GIL(SpawnActor))
    .def("try_spawn_actor", SPAWN_ACTOR_WITHOUT_GIL(TrySpawnActor))
    .def("wait_for_tick", &WaitForTick, (arg("seconds")=0.0))
    .def("on_tick", &OnTick, (arg("callback")))
    .def("remove_on_tick", &cc::World::RemoveOnTick, (arg("callback_id")))
    .def("tick", &Tick, (arg("seconds")=0.0))
    .def("set_pedestrians_cross_factor", CALL_WITHOUT_GIL_1(cc::World, SetPedestriansCrossFactor, float), (arg("percentage")))
    .def("set_pedestrians_seed", CALL_WITHOUT_GIL_1(cc::World, SetPedestriansSeed, unsigned int), (arg("seed")))
    .def("get_traffic_sign", CONST_CALL_WITHOUT_GIL_1(cc::World, GetTrafficSign, cc::Landmark), arg("landmark"))
    .def("get_traffic_light", CONST_CALL_WITHOUT_GIL_1(cc::World, GetTrafficLight, cc::Landmark), arg("landmark"))
    .def("get_traffic_light_from_opendrive_id", CONST_CALL_WITHOUT_GIL_1(cc::World, GetTrafficLightFromOpenDRIVE, const carla::road::SignId&), arg("traffic_light_id"))
    .def("get_traffic_lights_from_waypoint", CALL_RETURNING_LIST_2(cc::World, GetTrafficLightsFromWaypoint, const cc::Waypoint&, double), (arg("waypoint"), arg("distance")))
    .def("get_traffic_lights_in_junction", CALL_RETURNING_LIST_1(cc::World, GetTrafficLightsInJunction, carla::road::JuncId), (arg("junction_id")))
    .def("reset_all_traffic_lights", &cc::World::ResetAllTrafficLights)
    .def("get_lightmanager", CONST_CALL_WITHOUT_GIL(cc::World, GetLightManager))
    .def("freeze_all_traffic_lights", &cc::World::FreezeAllTrafficLights, (arg("frozen")))
    .def("get_level_bbs", &GetLevelBBs, (arg("bb_type")=cr::CityObjectLabel::Any))
    .def("get_environment_objects", &GetEnvironmentObjects, (arg("object_type")=cr::CityObjectLabel::Any))
    .def("enable_environment_objects", &EnableEnvironmentObjects, (arg("env_objects_ids"), arg("enable")))
    .def("cast_ray", CALL_RETURNING_LIST_2(cc::World, CastRay, cg::Location, cg::Location), (arg("initial_location"), arg("final_location")))
    .def("project_point", CALL_RETURNING_OPTIONAL_3(cc::World, ProjectPoint, cg::Location, cg::Vector3D, float), (arg("location"), arg("direction"), arg("search_distance")=10000.f))
    .def("ground_projection", CALL_RETURNING_OPTIONAL_2(cc::World, GroundProjection, cg::Location, float), (arg("location"), arg("search_distance")=10000.f))
    .def("get_names_of_all_objects", CALL_RETURNING_LIST(cc::World, GetNamesOfAllObjects))
    .def("apply_color_texture_to_object", &cc::World::ApplyColorTextureToObject, (arg("object_name"), arg("material_parameter"), arg("texture")))
    .def("apply_float_color_texture_to_object", &cc::World::ApplyFloatColorTextureToObject, (arg("object_name"), arg("material_parameter"), arg("texture")))
    .def("apply_textures_to_object", &cc::World::ApplyTexturesToObject, (arg("object_name"), arg("diffuse_texture"), arg("emissive_texture"), arg("normal_texture"), arg("ao_roughness_metallic_emissive_texture")))
    .def("apply_color_texture_to_objects", +[](cc::World &self, boost::python::list &list, const cr::MaterialParameter& parameter, const cr::TextureColor& Texture) {
        self.ApplyColorTextureToObjects(PythonLitstToVector<std::string>(list), parameter, Texture);
      }, (arg("objects_name_list"), arg("material_parameter"), arg("texture")))
    .def("apply_float_color_texture_to_objects", +[](cc::World &self, boost::python::list &list, const cr::MaterialParameter& parameter, const cr::TextureFloatColor& Texture) {
        self.ApplyFloatColorTextureToObjects(PythonLitstToVector<std::string>(list), parameter, Texture);
      }, (arg("objects_name_list"), arg("material_parameter"), arg("texture")))
    .def("apply_textures_to_objects", +[](cc::World &self, boost::python::list &list, const cr::TextureColor& diffuse_texture, const cr::TextureFloatColor& emissive_texture, const cr::TextureFloatColor& normal_texture, const cr::TextureFloatColor& ao_roughness_metallic_emissive_texture) {
        self.ApplyTexturesToObjects(PythonLitstToVector<std::string>(list), diffuse_texture, emissive_texture, normal_texture, ao_roughness_metallic_emissive_texture);
      }, (arg("objects_name_list"), arg("diffuse_texture"), arg("emissive_texture"), arg("normal_texture"), arg("ao_roughness_metallic_emissive_texture")))
    .def(self_ns::str(self_ns::self))
  ;

#undef SPAWN_ACTOR_WITHOUT_GIL

  class_<cc::DebugHelper>("DebugHelper", no_init)
    .def("draw_point", &cc::DebugHelper::DrawPoint,
        (arg("location"),
         arg("size")=0.1f,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
    .def("draw_line", &cc::DebugHelper::DrawLine,
        (arg("begin"),
         arg("end"),
         arg("thickness")=0.1f,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
    .def("draw_arrow", &cc::DebugHelper::DrawArrow,
        (arg("begin"),
         arg("end"),
         arg("thickness")=0.1f,
         arg("arrow_size")=0.1f,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
    .def("draw_box", &cc::DebugHelper::DrawBox,
        (arg("box"),
         arg("rotation"),
         arg("thickness")=0.1f,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
    .def("draw_string", &cc::DebugHelper::DrawString,
        (arg("location"),
         arg("text"),
         arg("draw_shadow")=false,
         arg("color")=cc::DebugHelper::Color(255u, 0u, 0u),
         arg("life_time")=-1.0f,
         arg("persistent_lines")=true))
  ;
}
