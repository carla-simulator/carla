// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PythonAPI.h>


#include <boost/python/stl_iterator.hpp>

#include <carla/rpc/ActorUpdateBatch.h>

namespace ctm = carla::traffic_manager;
namespace cg = carla::geom;

namespace carla {
namespace rpc {

  std::ostream &operator<<(std::ostream &out, const ServerPerfCounters &c) {
    out << "ServerPerfCounters(frames=" << c.frames
        << ", sync_calls=" << c.sync_calls
        << ", sync_call_ns=" << c.sync_call_ns
        << ", async_calls=" << c.async_calls
        << ", rpc_loop_ns=" << c.rpc_loop_ns
        << ", backlog_sum=" << c.backlog_sum
        << ", backlog_max=" << c.backlog_max
        << ", snapshots=" << c.snapshots
        << ", snapshot_ns=" << c.snapshot_ns
        << ", snapshot_bytes=" << c.snapshot_bytes
        << ", snapshot_writes=" << c.snapshot_writes
        << ", snapshot_wire_bytes=" << c.snapshot_wire_bytes
        << ", snapshot_drops=" << c.snapshot_drops
        << ", snapshot_blocked_ns=" << c.snapshot_blocked_ns
        << ", snapshot_queue_ns=" << c.snapshot_queue_ns
        << ", snapshot_send_ns=" << c.snapshot_send_ns
        << ", stream_writes=" << c.stream_writes
        << ", stream_wire_bytes=" << c.stream_wire_bytes
        << ", stream_drops=" << c.stream_drops
        << ", stream_blocked_ns=" << c.stream_blocked_ns
        << ", stream_queue_ns=" << c.stream_queue_ns
        << ", stream_send_ns=" << c.stream_send_ns
        << ", snapshot_encode_ns=" << c.snapshot_encode_ns << ")";
    return out;
  }

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

template <typename T>
static boost::python::list ToPythonList(const std::vector<T> &values) {
  boost::python::list result;
  for (const auto &value : values) {
    result.append(value);
  }
  return result;
}

static boost::python::dict StreamClassStatsToDict(const carla::rpc::StreamClassStats &s) {
  boost::python::dict d;
  d["sessions_opened"] = s.sessions_opened;
  d["sessions_closed"] = s.sessions_closed;
  d["writes"] = s.writes;
  d["wire_bytes"] = s.wire_bytes;
  d["busy_drops"] = s.busy_drops;
  d["closed_drops"] = s.closed_drops;
  d["timeouts"] = s.timeouts;
  d["strand_drops"] = s.strand_drops;
  d["write_errors"] = s.write_errors;
  d["drops"] = s.Drops();
  d["blocked_writes"] = s.blocked_writes;
  d["blocked_ns"] = s.blocked_ns;
  d["queue_ns"] = s.queue_ns;
  d["send_ns"] = s.send_ns;
  d["blocked_histogram"] = ToPythonList(s.blocked_histogram);
  d["queue_histogram"] = ToPythonList(s.queue_histogram);
  d["send_histogram"] = ToPythonList(s.send_histogram);
  return d;
}

static boost::python::dict GetStreamStats(const carla::client::Client &self) {
  carla::rpc::StreamStats stats;
  {
    carla::PythonUtil::ReleaseGIL unlock;
    stats = self.GetStreamStats();
  }
  boost::python::dict result;
  result["episode"] = StreamClassStatsToDict(stats.episode);
  result["other"] = StreamClassStatsToDict(stats.other);
  boost::python::list sessions;
  for (const auto &row : stats.sessions) {
    boost::python::dict d;
    d["stream_id"] = row.stream_id;
    d["session_id"] = row.session_id;
    d["episode"] = row.episode;
    d["remote"] = row.remote;
    d["loopback"] = row.loopback;
    d["writes"] = row.writes;
    d["wire_bytes"] = row.wire_bytes;
    d["busy_drops"] = row.busy_drops;
    d["closed_drops"] = row.closed_drops;
    d["timeouts"] = row.timeouts;
    d["blocked_ns"] = row.blocked_ns;
    d["queue_ns"] = row.queue_ns;
    d["send_ns"] = row.send_ns;
    sessions.append(d);
  }
  result["sessions"] = sessions;
  using Changes = carla::rpc::SnapshotChangeStats;
  const Changes &changes = stats.snapshot_changes;
  boost::python::dict c;
  c["frames"] = changes.frames;
  c["keyframes"] = changes.keyframes;
  c["removed"] = changes.removed;
  c["keyframe_bytes"] = changes.keyframe_bytes;
  c["delta_bytes"] = changes.delta_bytes;
  static const char *type_names[Changes::kTypeCount] = {
      "other", "vehicle", "walker", "traffic_light", "traffic_sign", "sensor"};
  boost::python::dict by_type;
  for (size_t t = 0u; t < Changes::kTypeCount && t < changes.by_type.size(); ++t) {
    const auto &row = changes.by_type[t];
    if (row.size() < Changes::kColumns) {
      continue;
    }
    boost::python::dict d;
    d["records"] = row[Changes::kRecords];
    d["unchanged"] = row[Changes::kUnchanged];
    d["added"] = row[Changes::kAdded];
    boost::python::list groups;
    for (size_t g = 0u; g < Changes::kGroupCount; ++g) {
      groups.append(row[Changes::kFirstGroup + g]);
    }
    d["group_changed"] = groups;
    by_type[type_names[t]] = d;
  }
  c["by_type"] = by_type;
  result["snapshot_changes"] = c;
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
  std::vector<std::string> files;
  {
    carla::PythonUtil::ReleaseGIL unlock;
    files = self.GetRequiredFiles(folder, download);
  }
  for (const auto &str : files) {
    result.append(str);
  }
  return result;
}

// Defined in ActorUpdates.cpp.
boost::python::object ApplyActorUpdates(
    const carla::client::Client &self,
    carla::rpc::ActorUpdateBatchBuilder &batch,
    bool do_tick);
void ApplyActorUpdatesAsync(
    const carla::client::Client &self,
    carla::rpc::ActorUpdateBatchBuilder &batch);

static void ApplyBatchCommands(
    const carla::client::Client &self,
    const boost::python::object &commands,
    bool do_tick) {
  using CommandType = carla::rpc::Command;
  std::vector<CommandType> cmds{
    boost::python::stl_input_iterator<CommandType>(commands),
        boost::python::stl_input_iterator<CommandType>()};
  carla::PythonUtil::ReleaseGIL unlock;
  self.ApplyBatch(std::move(cmds), do_tick);
}

static std::vector<carla::rpc::CommandResponse> ApplyBatchCommandsSyncNoGIL(
    const carla::client::Client &self,
    std::vector<carla::rpc::Command> &cmds,
    bool do_tick);

static auto ApplyBatchCommandsSync(
    const carla::client::Client &self,
    const boost::python::object &commands,
    bool do_tick) {

  using CommandType = carla::rpc::Command;
  std::vector<CommandType> cmds {
    boost::python::stl_input_iterator<CommandType>(commands),
    boost::python::stl_input_iterator<CommandType>()
  };

  // Release the GIL while waiting for the server: a sensor callback waiting
  // for the GIL stops its streaming thread from reading the socket, and in
  // synchronous mode the server then blocks sending to it (#10233).
  std::vector<carla::rpc::CommandResponse> responses;
  {
    carla::PythonUtil::ReleaseGIL unlock;
    responses = ApplyBatchCommandsSyncNoGIL(self, cmds, do_tick);
  }

  boost::python::list result;
  for (auto &response : responses) {
    result.append(std::move(response));
  }
  return result;
}

static std::vector<carla::rpc::CommandResponse> ApplyBatchCommandsSyncNoGIL(
    const carla::client::Client &self,
    std::vector<carla::rpc::Command> &cmds,
    bool do_tick) {

  using CommandType = carla::rpc::Command;
  auto responses = self.ApplyBatchSync(cmds, do_tick);

  // Find the SetAutopilot commands, on their own or in a SpawnActor's
  // do_after, of the commands that succeeded. Most batches have none.
  struct AutopilotRequest {
    carla::rpc::ActorId id;
    bool enabled;
  };
  std::vector<AutopilotRequest> requests;
  uint16_t tm_port = 8000;
  for (size_t i = 0; i < cmds.size() && i < responses.size(); ++i) {
    if (responses[i].HasError()) {
      continue;
    }
    const CommandType::CommandType &cmd_type = cmds[i].command;
    const carla::rpc::Command::SetAutopilot *autopilot = nullptr;
    if (const auto *spawn = std::get_if<carla::rpc::Command::SpawnActor>(&cmd_type)) {
      for (const auto &cmd : spawn->do_after) {
        if (const auto *maybe = std::get_if<carla::rpc::Command::SetAutopilot>(&cmd.command)) {
          autopilot = maybe;
        }
      }
    } else {
      autopilot = std::get_if<carla::rpc::Command::SetAutopilot>(&cmd_type);
    }
    if (autopilot != nullptr) {
      tm_port = autopilot->tm_port;
      requests.push_back({static_cast<carla::rpc::ActorId>(responses[i].Get()), autopilot->enabled});
    }
  }
  if (requests.empty()) {
    return responses;
  }

  carla::client::World world = self.GetWorld();
  std::vector<carla::traffic_manager::ActorPtr> vehicles_to_enable;
  std::vector<carla::traffic_manager::ActorPtr> vehicles_to_disable;
  for (const auto &request : requests) {
    carla::SharedPtr<carla::client::Actor> actor = world.GetActor(request.id);
    if (actor) {
      (request.enabled ? vehicles_to_enable : vehicles_to_disable).push_back(actor);
    }
  }

  // Ensure the TM always receives the same vector by sorting the elements
  const auto by_id = [](const carla::traffic_manager::ActorPtr &a, const carla::traffic_manager::ActorPtr &b) {
    return a->GetId() < b->GetId();
  };
  std::sort(vehicles_to_enable.begin(), vehicles_to_enable.end(), by_id);
  std::sort(vehicles_to_disable.begin(), vehicles_to_disable.end(), by_id);

  if (!vehicles_to_enable.empty() || !vehicles_to_disable.empty()) {
    self.GetInstanceTM(tm_port).RegisterVehicles(vehicles_to_enable);
    self.GetInstanceTM(tm_port).UnregisterVehicles(vehicles_to_disable);
  }

  return responses;
}

void export_client() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace rpc = carla::rpc;

  class_<rpc::ServerPerfCounters>("ServerPerfCounters", no_init)
    .def_readonly("frames", &rpc::ServerPerfCounters::frames)
    .def_readonly("sync_calls", &rpc::ServerPerfCounters::sync_calls)
    .def_readonly("sync_call_ns", &rpc::ServerPerfCounters::sync_call_ns)
    .def_readonly("async_calls", &rpc::ServerPerfCounters::async_calls)
    .def_readonly("rpc_loop_ns", &rpc::ServerPerfCounters::rpc_loop_ns)
    .def_readonly("backlog_sum", &rpc::ServerPerfCounters::backlog_sum)
    .def_readonly("backlog_max", &rpc::ServerPerfCounters::backlog_max)
    .def_readonly("snapshots", &rpc::ServerPerfCounters::snapshots)
    .def_readonly("snapshot_ns", &rpc::ServerPerfCounters::snapshot_ns)
    .def_readonly("snapshot_bytes", &rpc::ServerPerfCounters::snapshot_bytes)
    .def_readonly("snapshot_writes", &rpc::ServerPerfCounters::snapshot_writes)
    .def_readonly("snapshot_wire_bytes", &rpc::ServerPerfCounters::snapshot_wire_bytes)
    .def_readonly("snapshot_drops", &rpc::ServerPerfCounters::snapshot_drops)
    .def_readonly("snapshot_blocked_ns", &rpc::ServerPerfCounters::snapshot_blocked_ns)
    .def_readonly("snapshot_queue_ns", &rpc::ServerPerfCounters::snapshot_queue_ns)
    .def_readonly("snapshot_send_ns", &rpc::ServerPerfCounters::snapshot_send_ns)
    .def_readonly("stream_writes", &rpc::ServerPerfCounters::stream_writes)
    .def_readonly("stream_wire_bytes", &rpc::ServerPerfCounters::stream_wire_bytes)
    .def_readonly("stream_drops", &rpc::ServerPerfCounters::stream_drops)
    .def_readonly("stream_blocked_ns", &rpc::ServerPerfCounters::stream_blocked_ns)
    .def_readonly("stream_queue_ns", &rpc::ServerPerfCounters::stream_queue_ns)
    .def_readonly("stream_send_ns", &rpc::ServerPerfCounters::stream_send_ns)
    .def_readonly("snapshot_encode_ns", &rpc::ServerPerfCounters::snapshot_encode_ns)
    .def(self_ns::str(self_ns::self))
  ;

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
    .def("get_server_perf_counters", CONST_CALL_WITHOUT_GIL(cc::Client, GetServerPerfCounters))
    .def("get_stream_stats", &GetStreamStats)
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
    .def("apply_actor_updates", &ApplyActorUpdates, (arg("batch"), arg("do_tick")=false))
    .def("apply_actor_updates_async", &ApplyActorUpdatesAsync, (arg("batch")))
    .def("get_trafficmanager", CONST_CALL_WITHOUT_GIL_1(cc::Client, GetInstanceTM, uint16_t), (arg("port")=ctm::TM_DEFAULT_PORT))
  ;
}
