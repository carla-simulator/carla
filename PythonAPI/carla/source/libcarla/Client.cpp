// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/PythonUtil.h"
#include "carla/client/Client.h"
#include "carla/client/World.h"
#include "carla/Logging.h"
#include "carla/rpc/ActorId.h"
#include "carla/trafficmanager/TrafficManager.h"

#include <boost/python/stl_iterator.hpp>

static void SetTimeout(carla::client::Client &client, double seconds) {
  client.SetTimeout(TimeDurationFromSeconds(seconds));
}

static auto GetAvailableMaps(const carla::client::Client &self) {
  carla::PythonUtil::ReleaseGIL unlock;
  boost::python::list result;
  for (const auto &str : self.GetAvailableMaps()) {
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
        const boost::typeindex::type_info& cmd_type_info = cmd_type.type();

        // check SpawnActor command
        if (cmd_type_info == typeid(carla::rpc::Command::SpawnActor)) {
          // check inside 'do_after'
          auto &spawn = boost::get<carla::rpc::Command::SpawnActor>(cmd_type);
          for (auto &cmd : spawn.do_after) {
            if (cmd.command.type() == typeid(carla::rpc::Command::SetAutopilot)) {
              autopilotValue = boost::get<carla::rpc::Command::SetAutopilot>(cmd.command).enabled;
              isAutopilot = true;
            }
          }
        }
        // check SetAutopilot command
        else if (cmd_type_info == typeid(carla::rpc::Command::SetAutopilot)) {
          autopilotValue = boost::get<carla::rpc::Command::SetAutopilot>(cmd_type).enabled;
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

  // check if any autopilot command was sent
  if ((vehicles_to_enable.size() || vehicles_to_disable.size())) {
    self.GetInstanceTM().RegisterVehicles(vehicles_to_enable);
    self.GetInstanceTM().UnregisterVehicles(vehicles_to_disable);
  }

  return result;
}

void export_client() {
  using namespace boost::python;
  namespace cc = carla::client;

  class_<cc::Client>("Client",
      init<std::string, uint16_t, size_t>((arg("host"), arg("port"), arg("worker_threads")=0u)))
    .def("set_timeout", &::SetTimeout, (arg("seconds")))
    .def("get_client_version", &cc::Client::GetClientVersion)
    .def("get_server_version", CONST_CALL_WITHOUT_GIL(cc::Client, GetServerVersion))
    .def("get_world", &cc::Client::GetWorld)
    .def("get_available_maps", &GetAvailableMaps)
    .def("reload_world", CONST_CALL_WITHOUT_GIL(cc::Client, ReloadWorld))
    .def("load_world", CONST_CALL_WITHOUT_GIL_1(cc::Client, LoadWorld, std::string), (arg("map_name")))
    .def("generate_opendrive_world", CONST_CALL_WITHOUT_GIL_1(cc::Client, GenerateOpenDriveWorld, std::string), (arg("opendrive")))
    .def("start_recorder", CALL_WITHOUT_GIL_1(cc::Client, StartRecorder, std::string), (arg("name")))
    .def("stop_recorder", &cc::Client::StopRecorder)
    .def("show_recorder_file_info", CALL_WITHOUT_GIL_2(cc::Client, ShowRecorderFileInfo, std::string, bool), (arg("name"), arg("show_all")))
    .def("show_recorder_collisions", CALL_WITHOUT_GIL_3(cc::Client, ShowRecorderCollisions, std::string, char, char), (arg("name"), arg("type1"), arg("type2")))
    .def("show_recorder_actors_blocked", CALL_WITHOUT_GIL_3(cc::Client, ShowRecorderActorsBlocked, std::string, double, double), (arg("name"), arg("min_time"), arg("min_distance")))
    .def("replay_file", CALL_WITHOUT_GIL_4(cc::Client, ReplayFile, std::string, double, double, uint32_t), (arg("name"), arg("time_start"), arg("duration"), arg("follow_id")))
    .def("set_replayer_time_factor", &cc::Client::SetReplayerTimeFactor, (arg("time_factor")))
    .def("set_replayer_ignore_hero", &cc::Client::SetReplayerIgnoreHero, (arg("ignore_hero")))
    .def("apply_batch", &ApplyBatchCommands, (arg("commands"), arg("do_tick")=false))
    .def("apply_batch_sync", &ApplyBatchCommandsSync, (arg("commands"), arg("do_tick")=false))
    .def("get_trafficmanager", CONST_CALL_WITHOUT_GIL_1(cc::Client, GetInstanceTM, uint16_t), (arg("port")=TM_DEFAULT_PORT))
  ;
}
