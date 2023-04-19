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
#include <carla/client/GimbalSensor.h>
#include <carla/rpc/GimbalMode.h>

static void SubscribeToStream(carla::client::Sensor &self, boost::python::object callback) {
  self.Listen(MakeCallback(std::move(callback)));
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
  namespace cr = carla::rpc;

  class_<cc::Sensor, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Sensor>>("Sensor", no_init)
    .add_property("is_listening", &cc::Sensor::IsListening)
    .def("listen", &SubscribeToStream, (arg("callback")))
    .def("is_listening", &cc::Sensor::IsListening)
    .def("stop", &cc::Sensor::Stop)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::ServerSideSensor, bases<cc::Sensor>, boost::noncopyable, boost::shared_ptr<cc::ServerSideSensor>>
      ("ServerSideSensor", no_init)
    .def("listen_to_gbuffer", &SubscribeToGBuffer, (arg("gbuffer_id"), arg("callback")))
    .def("is_listening_gbuffer", &cc::ServerSideSensor::IsListeningGBuffer, (arg("gbuffer_id")))
    .def("stop_gbuffer", &cc::ServerSideSensor::StopGBuffer, (arg("gbuffer_id")))
    .def("set_gimbal_cmd", &cc::ServerSideSensor::SetGimbalCmd, (arg("cmd")))
    .def("set_gimbal_mode", &cc::ServerSideSensor::SetGimbalMode, (arg("mode")))
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

  enum_<cr::GimbalMode>("GimbalMode")
      .value("VehicleRelative", cr::GimbalMode::VehicleRelative)
      .value("WorldRelative", cr::GimbalMode::WorldRelative)
  ;

  class_<cc::GimbalSensor, bases<cc::Sensor>, boost::noncopyable, boost::shared_ptr<cc::GimbalSensor>>
      ("GimbalSensor", no_init)
    //.add_property("mode")
    .def("set_cmd", &cc::GimbalSensor::SetCmd, (arg("cmd")))
    .def("set_mode", &cc::GimbalSensor::SetMode, (arg("mode")))
    .def(self_ns::str(self_ns::self))
  ;
}
