// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/client/Sensor.h>

static void SubscribeToStream(carla::client::Sensor &self, boost::python::object callback) {
  namespace py = boost::python;
  // Make sure the callback is actually callable.
  if (!PyCallable_Check(callback.ptr())) {
    PyErr_SetString(PyExc_TypeError, "callback argument must be callable!");
    py::throw_error_already_set();
    return;
  }

  // We need to delete the callback while holding the GIL.
  using Deleter = carla::PythonUtil::AcquireGILDeleter;
  auto callback_ptr = carla::SharedPtr<py::object>{new py::object(callback), Deleter()};

  // Subscribe to the sensor.
  self.Listen([callback=std::move(callback_ptr)](auto message) {
    carla::PythonUtil::AcquireGIL lock;
    try {
      py::call<void>(callback->ptr(), py::object(message));
    } catch (const py::error_already_set &e) {
      PyErr_Print();
    }
  });
}

void export_sensor() {
  using namespace boost::python;
  namespace cc = carla::client;

  class_<cc::Sensor, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Sensor>>("Sensor", no_init)
    .add_property("is_listening", &cc::Sensor::IsListening)
    .def("listen", &SubscribeToStream, (arg("callback")))
    .def("stop", &cc::Sensor::Stop)
    .def(self_ns::str(self_ns::self))
  ;
}
