// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/client/ClientSideSensor.h>
#include <carla/client/LaneInvasionSensor.h>
#include <carla/client/Sensor.h>
#include <carla/client/ServerSideSensor.h>

static void SubscribeToStream(carla::client::Sensor &self, boost::python::object callback) {
  self.Listen(MakeCallback(std::move(callback)));
}

static void SetIgnoredVehiclesList(carla::client::ServerSideSensor &self, boost::python::list vehicle_ids) {
  self.SetIgnoredVehicles(PythonLitstToVector<carla::rpc::ActorId>(vehicle_ids));
}

static void SubscribeToGBuffer(
  carla::client::ServerSideSensor &self,
  uint32_t GBufferId,
  boost::python::object callback) {
  self.ListenToGBuffer(GBufferId, MakeCallback(std::move(callback)));
}

void export_sensor() {
  using namespace boost::python;
  namespace cc = carla::client;

  class_<cc::Sensor, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Sensor>>("Sensor", no_init)
    .def("listen", &SubscribeToStream, (arg("callback")))
    .def("is_listening", &cc::Sensor::IsListening)
    .def("stop", &cc::Sensor::Stop)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::ServerSideSensor, bases<cc::Sensor>, boost::noncopyable, boost::shared_ptr<cc::ServerSideSensor>>
      ("ServerSideSensor", no_init)
    .def("enable_gbuffers", &cc::ServerSideSensor::EnableGBuffers, (arg("enable")))
    .def("set_ignored_vehicles", &SetIgnoredVehiclesList, (arg("vehicle_ids")))
    .def("get_ignored_vehicles", &cc::ServerSideSensor::GetIgnoredVehicles)
    .def("listen_to_gbuffer", &SubscribeToGBuffer, (arg("gbuffer_id"), arg("callback")))
    .def("is_listening_gbuffer", &cc::ServerSideSensor::IsListeningGBuffer, (arg("gbuffer_id")))
    .def("stop_gbuffer", &cc::ServerSideSensor::StopGBuffer, (arg("gbuffer_id")))
    .def("enable_for_ros", &cc::ServerSideSensor::EnableForROS)
    .def("disable_for_ros", &cc::ServerSideSensor::DisableForROS)
    .def("is_enabled_for_ros", &cc::ServerSideSensor::IsEnabledForROS)
    .def("send", &cc::ServerSideSensor::Send, (arg("message")))
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::ClientSideSensor, bases<cc::Sensor>, boost::noncopyable, boost::shared_ptr<cc::ClientSideSensor>>
      ("ClientSideSensor", no_init)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::LaneInvasionSensor, bases<cc::ClientSideSensor>, boost::noncopyable, boost::shared_ptr<cc::LaneInvasionSensor>>
      ("LaneInvasionSensor", no_init)
    .def(self_ns::str(self_ns::self))
  ;

}
