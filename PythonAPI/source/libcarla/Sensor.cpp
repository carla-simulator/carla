// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Sensor.h>
#include <carla/image/ImageConverter.h>
#include <carla/image/ImageIO.h>
#include <carla/image/ImageView.h>
#include <carla/pointcloud/PLY.h>
#include <carla/sensor/SensorData.h>
#include <carla/sensor/data/Image.h>
#include <carla/sensor/data/LidarMeasurement.h>

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

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
    out << "Image(frame=" << image.GetFrameNumber()
        << ", size=" << image.GetWidth() << 'x' << image.GetHeight()
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const LidarMeasurement &meas) {
    out << "LidarMeasurement(frame=" << meas.GetFrameNumber()
        << ", number_of_points=" << meas.size()
        << ')';
    return out;
  }

} // namespace data
} // namespace sensor
} // namespace carla

enum class EColorConverter {
  None,
  Depth,
  LogarithmicDepth,
  CityScapesPalette
};

template <typename T>
static auto GetRawDataAsBuffer(T &self) {
  auto *data = reinterpret_cast<unsigned char *>(self.data());
  auto size = sizeof(typename T::value_type) * self.size();
#if PYTHON3X // NOTE(Andrei): python 3
  auto *ptr = PyMemoryView_FromMemory(reinterpret_cast<char *>(data), size, PyBUF_READ);
#else        // NOTE(Andrei): python 2
  auto *ptr = PyBuffer_FromMemory(data, size);
#endif
  return boost::python::object(boost::python::handle<>(ptr));
}

template <typename T>
static void ConvertImage(T &self, EColorConverter cc) {
  using namespace carla::image;
  auto view = ImageView::MakeView(self);
  switch (cc) {
    case EColorConverter::Depth:
      ImageConverter::ConvertInPlace(view, ColorConverter::Depth());
      break;
    case EColorConverter::LogarithmicDepth:
      ImageConverter::ConvertInPlace(view, ColorConverter::LogarithmicDepth());
      break;
    case EColorConverter::CityScapesPalette:
      ImageConverter::ConvertInPlace(view, ColorConverter::CityScapesPalette());
      break;
    case EColorConverter::None:
      break; // ignore.
    default:
      throw std::invalid_argument("invalid color converter!");
  }
}

template <typename T>
static void SaveImageToDisk(T &self, const std::string &path, EColorConverter cc) {
  using namespace carla::image;
  auto view = ImageView::MakeView(self);
  switch (cc) {
    case EColorConverter::None:
      ImageIO::WriteView(path, view);
      break;
    case EColorConverter::Depth:
      ImageIO::WriteView(path, ImageView::MakeColorConvertedView(view, ColorConverter::Depth()));
      break;
    case EColorConverter::LogarithmicDepth:
      ImageIO::WriteView(path, ImageView::MakeColorConvertedView(view, ColorConverter::LogarithmicDepth()));
      break;
    case EColorConverter::CityScapesPalette:
      ImageIO::WriteView(path, ImageView::MakeColorConvertedView(view, ColorConverter::CityScapesPalette()));
      break;
    default:
      throw std::invalid_argument("invalid color converter!");
  }
}

template <typename T>
static void SavePointCloudToDisk(T &self, const std::string &path) {
  carla::pointcloud::PLY::SaveToDisk(path, self.begin(), self.end());
}

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

  enum_<EColorConverter>("ColorConverter")
    .value("None", EColorConverter::None)
    .value("Depth", EColorConverter::Depth)
    .value("LogarithmicDepth", EColorConverter::LogarithmicDepth)
    .value("CityScapesPalette", EColorConverter::CityScapesPalette)
  ;

  class_<csd::Image, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::Image>>("Image", no_init)
    .add_property("width", &csd::Image::GetWidth)
    .add_property("height", &csd::Image::GetHeight)
    .add_property("fov", &csd::Image::GetFOVAngle)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::Image>)
    .def("convert", &ConvertImage<csd::Image>, (arg("color_converter")))
    .def("save_to_disk", &SaveImageToDisk<csd::Image>, (arg("path"), arg("color_converter")=EColorConverter::None))
    .def("__len__", &csd::Image::size)
    .def("__iter__", iterator<csd::Image>())
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::LidarMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::LidarMeasurement>>("LidarMeasurement", no_init)
    .add_property("horizontal_angle", &csd::LidarMeasurement::GetHorizontalAngle)
    .add_property("channels", &csd::LidarMeasurement::GetChannelCount)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::LidarMeasurement>)
    .def("get_point_count", &csd::LidarMeasurement::GetPointCount, (arg("channel")))
    .def("save_to_disk", &SavePointCloudToDisk<csd::LidarMeasurement>, (arg("path")))
    .def("__len__", &csd::LidarMeasurement::size)
    .def("__iter__", iterator<csd::LidarMeasurement>())
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
      self.Listen([callback](auto message) { // Here we retrieve the deserialized object.
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
