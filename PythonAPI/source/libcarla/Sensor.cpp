// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Sensor.h>
#include <carla/sensor/SensorData.h>
#include <carla/sensor/data/Image.h>

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

  std::ostream &operator<<(std::ostream &out, const Sensor &sensor) {
    out << "Sensor(id=" << sensor.GetId() << ", type=" << sensor.GetTypeId() << ')';
    return out;
  }

} // namespace client

namespace sensor {
namespace data {

  std::ostream &operator<<(std::ostream &out, const Image &image) {
    out << "Image(" << image.GetWidth() << 'x' << image.GetHeight()
        << ", frame=" << image.GetFrameNumber() << ')';
    return out;
  }

} // namespace data
} // namespace sensor
} // namespace carla

void export_sensor() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cs = carla::sensor;
  namespace csd = carla::sensor::data;

  class_<cs::SensorData, boost::noncopyable, boost::shared_ptr<cs::SensorData>>("SensorData", no_init)
    .add_property("frame_number", &cs::SensorData::GetFrameNumber)
    .add_property("transform", +[](const cs::SensorData &self) -> carla::rpc::Transform {
      return self.GetSensorTransform();
    })
  ;

  class_<csd::Image, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::Image>>("Image", no_init)
    .add_property("width", &csd::Image::GetWidth)
    .add_property("height", &csd::Image::GetHeight)
    .add_property("fov", &csd::Image::GetFOVAngle)
    .add_property("raw_data", +[](csd::Image &self) {
      // An image is an Array<Color>, convert back to buffer of chars.
      auto *data = reinterpret_cast<unsigned char *>(self.data());
      auto size = sizeof(csd::Image::value_type) * self.size();
#if PYTHON3X // NOTE(Andrei): python37
      auto *ptr = PyMemoryView_FromMemory(reinterpret_cast<char *>(data), size, PyBUF_READ);
#else        // NOTE(Andrei): python2.7
      auto *ptr = PyBuffer_FromMemory(data, size);
#endif
      return object(handle<>(ptr));
    })
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
      self.Listen([callback](auto message) { // Here we retrieve already the deserialized object.
        GILLockGuard gil_lock;
        try {
          boost::python::call<void>(callback.ptr(), boost::python::object(message));
        } catch (const boost::python::error_already_set &e) {
          PyErr_Print();
        }
      });
    }, (arg("callback")))
    .def(self_ns::str(self_ns::self))
  ;
}
