// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/client/Client.h>
#include <carla/client/World.h>

static void SetTimeout(carla::client::Client &client, double seconds) {
  size_t ms = static_cast<size_t>(1e3 * seconds);
  client.SetTimeout(carla::time_duration::milliseconds(ms));
}

void export_client() {
  using namespace boost::python;
  namespace cc = carla::client;

  class_<cc::Client>("Client",
      init<std::string, uint16_t, size_t>((arg("host"), arg("port"), arg("worker_threads")=0u)))
    .def("set_timeout", &::SetTimeout, (arg("seconds")))
    .def("get_client_version", &cc::Client::GetClientVersion)
    .def("get_server_version", CONST_CALL_WITHOUT_GIL(cc::Client, GetServerVersion))
    .def("ping", CONST_CALL_WITHOUT_GIL(cc::Client, Ping))
    .def("get_world", &cc::Client::GetWorld)
  ;
}
