// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PythonAPI.h>

#include <thread>

#include <boost/python/stl_iterator.hpp>

namespace ctm = carla::traffic_manager;

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

  // check for autopilot command
  std::vector<carla::traffic_manager::ActorPtr> vehicles_to_enable(cmds.size(), nullptr);
  std::vector<carla::traffic_manager::ActorPtr> vehicles_to_disable(cmds.size(), nullptr);
  carla::client::World world = self.GetWorld();
  uint16_t tm_port = 8000;

  std::atomic<size_t> vehicles_to_enable_index;
  std::atomic<size_t> vehicles_to_disable_index;

  vehicles_to_enable_index.store(0);
  vehicles_to_disable_index.store(0);

  auto ProcessCommand = [&](size_t min_index, size_t max_index) {
    for (size_t i = min_index; i < max_index; ++i) {
      if (!responses[i].HasError()) {

        bool isAutopilot = false;
        bool autopilotValue = false;

        CommandType::CommandType& cmd_type = cmds[i].command;

        // check SpawnActor command
        if (const auto *maybe_spawn_actor_cmd = std::get_if<carla::rpc::Command::SpawnActor>(&cmd_type)) {
          // check inside 'do_after'
          for (auto &cmd : maybe_spawn_actor_cmd->do_after) {
            if (const auto *maybe_set_autopilot_command = std::get_if<carla::rpc::Command::SetAutopilot>(&cmd.command)) {
              tm_port = maybe_set_autopilot_command->tm_port;
              autopilotValue = maybe_set_autopilot_command->enabled;
              isAutopilot = true;
            }
          }
        }
        // check SetAutopilot command
        else if (const auto *maybe_set_autopilot_command = std::get_if<carla::rpc::Command::SetAutopilot>(&cmd_type)) {
          tm_port = maybe_set_autopilot_command->tm_port;
          autopilotValue = maybe_set_autopilot_command->enabled;
          isAutopilot = true;
        }

        // check if found any SetAutopilot command
        if (isAutopilot) {
          // get the id
          carla::rpc::ActorId id = static_cast<carla::rpc::ActorId>(responses[i].Get());

          // get all actors
          carla::SharedPtr<carla::client::Actor> actor;
          actor = world.GetActor(id);

          // check to enable or disable
          if (actor) {
            if (autopilotValue) {
              size_t index = vehicles_to_enable_index.fetch_add(1);
              vehicles_to_enable[index] = actor;
            } else {
              size_t index = vehicles_to_disable_index.fetch_add(1);
              vehicles_to_disable[index] = actor;
            }
          }
        }
      }
    }
  };

  const size_t TaskLimit = 50;
  size_t num_commands = cmds.size();
  size_t num_batches = num_commands / TaskLimit;

  std::vector<std::thread*> t(num_batches+1);

  for(size_t n = 0; n < num_batches; n++) {
    t[n] = new std::thread(ProcessCommand, n * TaskLimit, (n+1) * TaskLimit);
  }
  t[num_batches] = new std::thread(ProcessCommand, num_batches * TaskLimit, num_commands);

  for(size_t n = 0; n <= num_batches; n++) {
    if(t[n]->joinable()){
      t[n]->join();
    }
    delete t[n];
  }

  // Fix vector size
  vehicles_to_enable.resize(vehicles_to_enable_index.load());
  vehicles_to_disable.resize(vehicles_to_disable_index.load());
  // Release memory
  vehicles_to_enable.shrink_to_fit();
  vehicles_to_disable.shrink_to_fit();

  // Ensure the TM always receives the same vector by sorting the elements
  std::vector<carla::traffic_manager::ActorPtr> sorted_vehicle_to_enable = vehicles_to_enable;
  std::sort(sorted_vehicle_to_enable.begin(), sorted_vehicle_to_enable.end(), [](carla::traffic_manager::ActorPtr &a, carla::traffic_manager::ActorPtr &b) {return a->GetId() < b->GetId(); });

  std::vector<carla::traffic_manager::ActorPtr> sorted_vehicle_to_disable = vehicles_to_disable;
  std::sort(sorted_vehicle_to_disable.begin(), sorted_vehicle_to_disable.end(), [](carla::traffic_manager::ActorPtr &a, carla::traffic_manager::ActorPtr &b) {return a->GetId() < b->GetId(); });

  // check if any autopilot command was sent
  if (sorted_vehicle_to_enable.size() || sorted_vehicle_to_disable.size()) {
    self.GetInstanceTM(tm_port).RegisterVehicles(sorted_vehicle_to_enable);
    self.GetInstanceTM(tm_port).UnregisterVehicles(sorted_vehicle_to_disable);
  }

  return result;
}

void export_client() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace rpc = carla::rpc;

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

  class_<cc::Client>("Client",
      init<std::string, uint16_t, size_t>((arg("host")="127.0.0.1", arg("port")=2000, arg("worker_threads")=0u)))
    .def("set_timeout", &::SetTimeout, (arg("seconds")))
    .def("get_client_version", &cc::Client::GetClientVersion)
    .def("get_server_version", CONST_CALL_WITHOUT_GIL(cc::Client, GetServerVersion))
    .def("get_world", &cc::Client::GetWorld)
    .def("get_available_maps", &GetAvailableMaps)
    .def("set_files_base_folder", &cc::Client::SetFilesBaseFolder, (arg("path")))
    .def("get_required_files", &GetRequiredFiles, (arg("folder")="", arg("download")=true))
    .def("request_file", &cc::Client::RequestFile, (arg("name")))
    .def("reload_world", CONST_CALL_WITHOUT_GIL_1(cc::Client, ReloadWorld, bool), (arg("reset_settings")=true))
    .def("load_world", CONST_CALL_WITHOUT_GIL_3(cc::Client, LoadWorld, std::string, bool, rpc::MapLayer), (arg("map_name"), arg("reset_settings")=true, arg("map_layers")=rpc::MapLayer::All))
    .def("load_world_if_different", &cc::Client::LoadWorldIfDifferent, (arg("map_name"), arg("reset_settings")=true, arg("map_layers")=rpc::MapLayer::All))
    .def("generate_opendrive_world", CONST_CALL_WITHOUT_GIL_3(cc::Client, GenerateOpenDriveWorld, std::string,
        rpc::OpendriveGenerationParameters, bool), (arg("opendrive"), arg("parameters")=rpc::OpendriveGenerationParameters(),
        arg("reset_settings")=true))
    .def("start_recorder", CALL_WITHOUT_GIL_2(cc::Client, StartRecorder, std::string, bool), (arg("name"), arg("additional_data")=false))
    .def("stop_recorder", &cc::Client::StopRecorder)
    .def("show_recorder_file_info", CALL_WITHOUT_GIL_2(cc::Client, ShowRecorderFileInfo, std::string, bool), (arg("name"), arg("show_all")))
    .def("show_recorder_collisions", CALL_WITHOUT_GIL_3(cc::Client, ShowRecorderCollisions, std::string, char, char), (arg("name"), arg("type1"), arg("type2")))
    .def("show_recorder_actors_blocked", CALL_WITHOUT_GIL_3(cc::Client, ShowRecorderActorsBlocked, std::string, double, double), (arg("name"), arg("min_time"), arg("min_distance")))
    .def("replay_file", CALL_WITHOUT_GIL_5(cc::Client, ReplayFile, std::string, double, double, uint32_t, bool), (arg("name"), arg("time_start"), arg("duration"), arg("follow_id"), arg("replay_sensors")=false))
    .def("stop_replayer", &cc::Client::StopReplayer, (arg("keep_actors")))
    .def("set_replayer_time_factor", &cc::Client::SetReplayerTimeFactor, (arg("time_factor")))
    .def("set_replayer_ignore_hero", &cc::Client::SetReplayerIgnoreHero, (arg("ignore_hero")))
    .def("set_replayer_ignore_spectator", &cc::Client::SetReplayerIgnoreSpectator, (arg("ignore_spectator")))
    .def("apply_batch", &ApplyBatchCommands, (arg("commands"), arg("do_tick")=false))
    .def("apply_batch_sync", &ApplyBatchCommandsSync, (arg("commands"), arg("do_tick")=false))
    .def("get_trafficmanager", CONST_CALL_WITHOUT_GIL_1(cc::Client, GetInstanceTM, uint16_t), (arg("port")=ctm::TM_DEFAULT_PORT))
  ;
}
