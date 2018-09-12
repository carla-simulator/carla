// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Actor.h>
#include <carla/client/Image.h>
#include <carla/client/Sensor.h>
#include <carla/client/Vehicle.h>

#include <boost/python.hpp>

#include <ostream>
#include <iostream>

/// Aquires a lock on the Python's Global Interpreter Lock, necessary for
/// calling Python code from a different thread.
///
/// https://wiki.python.org/moin/GlobalInterpreterLock
class GILLockGuard {
public:

  GILLockGuard()
    : _state(PyGILState_Ensure()) {}

  ~GILLockGuard() {
    PyGILState_Release(_state);
  }

private:

  PyGILState_STATE _state;
};

namespace carla {
namespace client {

  std::ostream &operator<<(std::ostream &out, const Actor &actor) {
    out << "Actor(id=" << actor.GetId() << ", type=" << actor.GetTypeId() << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Vehicle &vehicle) {
    out << "Vehicle(id=" << vehicle.GetId() << ", type=" << vehicle.GetTypeId() << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Sensor &sensor) {
    out << "Sensor(id=" << sensor.GetId() << ", type=" << sensor.GetTypeId() << ')';
    return out;
  }

} // namespace client
} // namespace carla

void export_actor() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cr = carla::rpc;

  class_<cc::Image, boost::noncopyable, boost::shared_ptr<cc::Image>>("Image", no_init)
    .add_property("frame_number", &cc::Image::GetFrameNumber)
    .add_property("width", &cc::Image::GetWidth)
    .add_property("height", &cc::Image::GetHeight)
    .add_property("type", +[](const cc::Image &self) -> std::string {
      return self.GetType();
    })
    .add_property("fov", &cc::Image::GetFOV)
    .add_property("raw_data", +[](cc::Image &self) {

#if PYTHON3X
      //NOTE(Andrei): python37
      auto *ptr = PyMemoryView_FromMemory(reinterpret_cast<char *>(self.GetData()), self.GetSize(), PyBUF_READ);
#else
      //NOTE(Andrei): python2.7
      auto *ptr = PyBuffer_FromMemory(self.GetData(), self.GetSize());
#endif
      return object(handle<>(ptr));
    })
  ;

  class_<cc::Actor, boost::noncopyable, boost::shared_ptr<cc::Actor>>("Actor", no_init)
    .add_property("id", &cc::Actor::GetId)
    .add_property("type_id", +[](const cc::Actor &self) -> std::string {
      return self.GetTypeId();
    })
    .def("get_world", &cc::Actor::GetWorld)
    .def("get_location", &cc::Actor::GetLocation)
    .def("get_transform", &cc::Actor::GetTransform)
    .def("set_location", &cc::Actor::SetLocation, (arg("location")))
    .def("set_transform", &cc::Actor::SetTransform, (arg("transform")))
    .def("destroy", &cc::Actor::Destroy)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Vehicle, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Vehicle>>("Vehicle", no_init)
    .def("apply_control", &cc::Vehicle::ApplyControl, (arg("control")))
    .def("set_autopilot", &cc::Vehicle::SetAutopilot, (arg("enabled")=true))
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::Sensor, bases<cc::Actor>, boost::noncopyable, boost::shared_ptr<cc::Sensor>>("Sensor", no_init)
    .def("listen", +[](cc::Sensor &self, boost::python::object callback) {
      // Make sure the callback is actually callable.
      if (!PyCallable_Check(callback.ptr())) {
        PyErr_SetString(PyExc_TypeError, "callback argument must be callable!");
        throw_error_already_set();
        return;
      }

      // Subscribe to the sensor.
      self.Listen([callback](auto message) {
        cc::SharedPtr<cc::Image> image;
        try {
          image = cc::Image::FromBuffer(std::move(message));
        } catch (const std::exception &e) {
          std::cerr << "exception while parsing the image: " << e.what() << std::endl;
          return;
        }

        GILLockGuard gil_lock;
        try {
          boost::python::call<void>(callback.ptr(), boost::python::object(image));
        } catch (const boost::python::error_already_set &e) {
          PyErr_Print();
        }
      });
    }, (arg("callback")))
    .def(self_ns::str(self_ns::self))
  ;
}
