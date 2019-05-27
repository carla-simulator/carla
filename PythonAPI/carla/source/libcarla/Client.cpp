// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/client/Client.h>
#include <carla/client/World.h>

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
  std::vector<CommandType> cmds{
      boost::python::stl_input_iterator<CommandType>(commands),
      boost::python::stl_input_iterator<CommandType>()};
  boost::python::list result;
  for (auto &response : self.ApplyBatchSync(std::move(cmds), do_tick)) {
    result.append(std::move(response));
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
    .def("start_recorder", CALL_WITHOUT_GIL_1(cc::Client, StartRecorder, std::string), (arg("name")))
    .def("stop_recorder", &cc::Client::StopRecorder)
    .def("show_recorder_file_info", CALL_WITHOUT_GIL_2(cc::Client, ShowRecorderFileInfo, std::string, bool), (arg("name"), arg("show_all")))
    .def("show_recorder_collisions", CALL_WITHOUT_GIL_3(cc::Client, ShowRecorderCollisions, std::string, char, char), (arg("name"), arg("type1"), arg("type2")))
    .def("show_recorder_actors_blocked", CALL_WITHOUT_GIL_3(cc::Client, ShowRecorderActorsBlocked, std::string, double, double), (arg("name"), arg("min_time"), arg("min_distance")))
    .def("replay_file", CALL_WITHOUT_GIL_4(cc::Client, ReplayFile, std::string, double, double, uint32_t), (arg("name"), arg("time_start"), arg("duration"), arg("follow_id")))
    .def("set_replayer_time_factor", &cc::Client::SetReplayerTimeFactor, (arg("time_factor")))
    .def("apply_batch", &ApplyBatchCommands, (arg("commands"), arg("do_tick")=false))
    .def("apply_batch_sync", &ApplyBatchCommandsSync, (arg("commands"), arg("do_tick")=false))
    .def("create_walker", CALL_RETURNING_LIST_2(cc::Client, CreateWalker, carla::geom::Location, carla::geom::Location), (arg("location"), arg("location")))
  ;
}
