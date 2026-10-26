// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PythonAPI.h>

#include <boost/python/stl_iterator.hpp>

namespace ctm = carla::traffic_manager;
namespace cg = carla::geom;

namespace carla {
namespace rpc {

  std::ostream &operator<<(std::ostream &out, const ContentPackInfo &pack) {
    out << "ContentPackInfo(name=" << pack.name
        << ", version=" << pack.version
        << ", base_release=" << pack.base_release
        << ", path=" << pack.path
        << ", mounted=" << (pack.mounted ? "True" : "False")
        << ", maps=[";
    for (size_t i = 0u; i < pack.maps.size(); ++i) {
      out << (i == 0u ? "" : ", ") << pack.maps[i];
    }
    out << "])";
    return out;
  }

} // namespace rpc
} // namespace carla

static void SetTimeout(carla::client::Client &client, double seconds) {
  client.SetTimeout(TimeDurationFromSeconds(seconds));
}

static auto GetAvailableMaps(const carla::client::Client &self) {
  boost::python::list result;
  std::vector<std::string> maps;
  {
    carla::PythonUtil::ReleaseGIL unlock;
    maps = self.GetAvailableMaps();
  }
  for (const auto &str : maps) {
    result.append(str);
  }
  return result;
}

static auto GetContentPacks(const carla::client::Client &self) {
  boost::python::list result;
  std::vector<carla::rpc::ContentPackInfo> packs;
  {
    carla::PythonUtil::ReleaseGIL unlock;
    packs = self.GetContentPacks();
  }
  for (const auto &pack : packs) {
    result.append(pack);
  }
  return result;
}

static auto ContentPackInfoMaps(const carla::rpc::ContentPackInfo &self) {
  boost::python::list result;
  for (const auto &map : self.maps) {
    result.append(map);
  }
  return result;
}

static auto GetRequiredFiles(const carla::client::Client &self, const std::string &folder, const bool download) {
  boost::python::list result;
  for (const auto &str : self.GetRequiredFiles(folder, download)) {
    result.append(str);
  }
  return result;
}

static void ApplyBatchCommands(
    const carla::client::Client &self,
    const boost::python::object &commands,
    bool do_tick) {
  using CommandType = carla::rpc::Command;
  std::vector<CommandType> cmds{
    boost::python::stl_input_iterator<CommandType>(commands),
        boost::python::stl_input_iterator<CommandType>()};
  self.ApplyBatch(std::move(cmds), do_tick);
}

static auto ApplyBatchCommandsSync(
    const carla::client::Client &self,
    const boost::python::object &commands,
    bool do_tick) {

  using CommandType = carla::rpc::Command;
  std::vector<CommandType> cmds {
    boost::python::stl_input_iterator<CommandType>(commands),
    boost::python::stl_input_iterator<CommandType>()
  };

  boost::python::list result;
  auto responses = self.ApplyBatchSync(cmds, do_tick);
  for (auto &response : responses) {
    result.append(std::move(response));
  }

  return result;
}

void export_client() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace rpc = carla::rpc;

  class_<rpc::ContentPackInfo>("ContentPackInfo", no_init)
    .def_readonly("name", &rpc::ContentPackInfo::name)
    .def_readonly("version", &rpc::ContentPackInfo::version)
    .def_readonly("base_release", &rpc::ContentPackInfo::base_release)
    .def_readonly("path", &rpc::ContentPackInfo::path)
    .def_readonly("mounted", &rpc::ContentPackInfo::mounted)
    .add_property("maps", &ContentPackInfoMaps)
    .def(self_ns::str(self_ns::self))
  ;

  class_<rpc::OpendriveGenerationParameters>("OpendriveGenerationParameters",
      init<double, double, double, double, bool, bool, bool>((arg("vertex_distance")=2.0, arg("max_road_length")=50.0, arg("wall_height")=1.0, arg("additional_width")=0.6, arg("smooth_junctions")=true, arg("enable_mesh_visibility")=true, arg("enable_pedestrian_navigation")=true)))
    .def_readwrite("vertex_distance", &rpc::OpendriveGenerationParameters::vertex_distance)
    .def_readwrite("max_road_length", &rpc::OpendriveGenerationParameters::max_road_length)
    .def_readwrite("wall_height", &rpc::OpendriveGenerationParameters::wall_height)
    .def_readwrite("additional_width", &rpc::OpendriveGenerationParameters::additional_width)
    .def_readwrite("smooth_junctions", &rpc::OpendriveGenerationParameters::smooth_junctions)
    .def_readwrite("enable_mesh_visibility", &rpc::OpendriveGenerationParameters::enable_mesh_visibility)
    .def_readwrite("enable_pedestrian_navigation", &rpc::OpendriveGenerationParameters::enable_pedestrian_navigation)
  ;

#define REPLAY_FILE_WITHOUT_GIL(fn) +[]( \
        cc::Client &self, \
        std::string name, \
        double time_start, \
        double duration, \
        uint32_t follow_id, \
        bool replay_sensors, \
        bool replay_weather, \
        cg::Transform offset, \
        std::string map_override) { \
      carla::PythonUtil::ReleaseGIL unlock; \
      return self.fn(name, time_start, duration, follow_id, replay_sensors, replay_weather, offset, map_override); \
    }, \
    ( \
      arg("name"), \
      arg("time_start"), \
      arg("duration"), \
      arg("follow_id"), \
      arg("replay_sensors")=false, \
      arg("replay_weather")=false, \
      arg("offset")=cg::Transform(cg::Location(cg::Vector3D(-10, 0, 5)), cg::Rotation(-25, 0, 0)), \
      arg("map_override")="")

  class_<cc::Client>("Client",
      init<std::string, uint16_t, size_t>((arg("host")="127.0.0.1", arg("port")=2000, arg("worker_threads")=0u)))
    .def("set_timeout", &::SetTimeout, (arg("seconds")))
    .def("get_client_version", &cc::Client::GetClientVersion)
    .def("get_server_version", CONST_CALL_WITHOUT_GIL(cc::Client, GetServerVersion))
    .def("get_world", &cc::Client::GetWorld)
    .def("get_available_maps", &GetAvailableMaps)
    .def("get_content_packs", &GetContentPacks)
    .def("mount_content_pack", CONST_CALL_WITHOUT_GIL_1(cc::Client, MountContentPack, std::string), (arg("path")))
    .def("unmount_content_pack", CONST_CALL_WITHOUT_GIL_1(cc::Client, UnmountContentPack, std::string), (arg("name")))
    .def("set_files_base_folder", &cc::Client::SetFilesBaseFolder, (arg("path")))
    .def("get_required_files", &GetRequiredFiles, (arg("folder")="", arg("download")=true))
    .def("request_file", &cc::Client::RequestFile, (arg("name")))
    .def("reload_world", CONST_CALL_WITHOUT_GIL_1(cc::Client, ReloadWorld, bool), (arg("reset_settings")=true))
    .def("load_world", CONST_CALL_WITHOUT_GIL_3(cc::Client, LoadWorld, std::string, bool, rpc::MapLayer), (arg("map_name"), arg("reset_settings")=true, arg("map_layers")=rpc::MapLayer::All))
    .def("load_world_if_different", CONST_CALL_WITHOUT_GIL_3(cc::Client, LoadWorldIfDifferent, std::string, bool, rpc::MapLayer), (arg("map_name"), arg("reset_settings")=true, arg("map_layers")=rpc::MapLayer::All))
    .def("generate_opendrive_world", CONST_CALL_WITHOUT_GIL_3(cc::Client, GenerateOpenDriveWorld, std::string,
        rpc::OpendriveGenerationParameters, bool), (arg("opendrive"), arg("parameters")=rpc::OpendriveGenerationParameters(),
        arg("reset_settings")=true))
    .def("start_recorder", CALL_WITHOUT_GIL_3(cc::Client, StartRecorder, std::string, bool, bool), (arg("name"), arg("additional_data")=false, arg("stop_replayer")=true))
    .def("stop_recorder", &cc::Client::StopRecorder)
    .def("show_recorder_file_info", CALL_WITHOUT_GIL_2(cc::Client, ShowRecorderFileInfo, std::string, bool), (arg("name"), arg("show_all")))
    .def("show_recorder_collisions", CALL_WITHOUT_GIL_3(cc::Client, ShowRecorderCollisions, std::string, char, char), (arg("name"), arg("type1"), arg("type2")))
    .def("show_recorder_actors_blocked", CALL_WITHOUT_GIL_3(cc::Client, ShowRecorderActorsBlocked, std::string, double, double), (arg("name"), arg("min_time"), arg("min_distance")))
    .def("replay_file", REPLAY_FILE_WITHOUT_GIL(ReplayFile))
    .def("stop_replayer", &cc::Client::StopReplayer, (arg("keep_actors")))
    .def("set_replayer_time_factor", &cc::Client::SetReplayerTimeFactor, (arg("time_factor")))
    .def("set_replayer_ignore_hero", &cc::Client::SetReplayerIgnoreHero, (arg("ignore_hero")))
    .def("set_replayer_ignore_spectator", &cc::Client::SetReplayerIgnoreSpectator, (arg("ignore_spectator")))
    .def("apply_batch", &ApplyBatchCommands, (arg("commands"), arg("do_tick")=false))
    .def("apply_batch_sync", &ApplyBatchCommandsSync, (arg("commands"), arg("do_tick")=false))
    .def("get_trafficmanager", CONST_CALL_WITHOUT_GIL_1(cc::Client, GetInstanceTM, uint16_t), (arg("port")=ctm::TM_DEFAULT_PORT))
  ;
}
