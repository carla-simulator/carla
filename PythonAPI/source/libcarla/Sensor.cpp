// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/client/Sensor.h>

static void SubscribeToStream(carla::client::Sensor &self, boost::python::object callback) {
  // Make sure the callback is actually callable.
  if (!PyCallable_Check(callback.ptr())) {
    PyErr_SetString(PyExc_TypeError, "callback argument must be callable!");
    boost::python::throw_error_already_set();
    return;
  }

  // Subscribe to the sensor.
  self.Listen([callback](auto message) {
    carla::PythonUtil::AcquireGIL lock;
    try {
      boost::python::call<void>(callback.ptr(), boost::python::object(message));
    } catch (const boost::python::error_already_set &e) {
      PyErr_Print();
    }
  });
}

void export_sensor() {
  using namespace boost::python;
  namespace cc = carla::client;

  class_<cc::Sensor, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Sensor>>("Sensor", no_init)
    .def("listen", &SubscribeToStream, (arg("callback")))
    .def(self_ns::str(self_ns::self))
  ;
}
