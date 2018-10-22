// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/image/ImageConverter.h>
#include <carla/image/ImageIO.h>
#include <carla/image/ImageView.h>
#include <carla/pointcloud/PointCloudIO.h>
#include <carla/sensor/SensorData.h>
#include <carla/sensor/data/CollisionEvent.h>
#include <carla/sensor/data/Image.h>
#include <carla/sensor/data/LidarMeasurement.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <ostream>
#include <iostream>

namespace carla {
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

  std::ostream &operator<<(std::ostream &out, const CollisionEvent &meas) {
    out << "CollisionEvent(frame=" << meas.GetFrameNumber()
        << ", other_actor=" << meas.GetOtherActor()
        << ')';
    return out;
  }

} // namespace data
} // namespace sensor
} // namespace carla

enum class EColorConverter {
  Raw,
  Depth,
  LogarithmicDepth,
  CityScapesPalette
};

template <typename T>
static auto GetRawDataAsBuffer(T &self) {
  auto *data = reinterpret_cast<unsigned char *>(self.data());
  auto size = sizeof(typename T::value_type) * self.size();
#if PY_MAJOR_VERSION >= 3 // NOTE(Andrei): python 3
  auto *ptr = PyMemoryView_FromMemory(reinterpret_cast<char *>(data), size, PyBUF_READ);
#else        // NOTE(Andrei): python 2
  auto *ptr = PyBuffer_FromMemory(data, size);
#endif
  return boost::python::object(boost::python::handle<>(ptr));
}

template <typename T>
static void ConvertImage(T &self, EColorConverter cc) {
  carla::PythonUtil::ReleaseGIL unlock;
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
    case EColorConverter::Raw:
      break; // ignore.
    default:
      throw std::invalid_argument("invalid color converter!");
  }
}

template <typename T>
static std::string SaveImageToDisk(T &self, std::string path, EColorConverter cc) {
  carla::PythonUtil::ReleaseGIL unlock;
  using namespace carla::image;
  auto view = ImageView::MakeView(self);
  switch (cc) {
    case EColorConverter::Raw:
      return ImageIO::WriteView(
          std::move(path),
          view);
    case EColorConverter::Depth:
      return ImageIO::WriteView(
          std::move(path),
          ImageView::MakeColorConvertedView(view, ColorConverter::Depth()));
    case EColorConverter::LogarithmicDepth:
      return ImageIO::WriteView(
          std::move(path),
          ImageView::MakeColorConvertedView(view, ColorConverter::LogarithmicDepth()));
    case EColorConverter::CityScapesPalette:
      return ImageIO::WriteView(
          std::move(path),
          ImageView::MakeColorConvertedView(view, ColorConverter::CityScapesPalette()));
    default:
      throw std::invalid_argument("invalid color converter!");
  }
}

template <typename T>
static std::string SavePointCloudToDisk(T &self, std::string path) {
  carla::PythonUtil::ReleaseGIL unlock;
  return carla::pointcloud::PointCloudIO::SaveToDisk(std::move(path), self.begin(), self.end());
}

void export_sensor_data() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cr = carla::rpc;
  namespace cs = carla::sensor;
  namespace csd = carla::sensor::data;

  class_<cs::SensorData, boost::noncopyable, boost::shared_ptr<cs::SensorData>>("SensorData", no_init)
    .add_property("frame_number", &cs::SensorData::GetFrameNumber)
    .add_property("transform", CALL_RETURNING_COPY(cs::SensorData, GetSensorTransform))
  ;

  enum_<EColorConverter>("ColorConverter")
    .value("Raw", EColorConverter::Raw)
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
    .def("save_to_disk", &SaveImageToDisk<csd::Image>, (arg("path"), arg("color_converter")=EColorConverter::Raw))
    .def("__len__", &csd::Image::size)
    .def("__iter__", iterator<csd::Image>())
    .def("__getitem__", +[](const csd::Image &self, size_t pos) -> csd::Color {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::Image &self, size_t pos, csd::Color color) {
      self.at(pos) = color;
    })
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
    .def("__getitem__", +[](const csd::LidarMeasurement &self, size_t pos) -> cr::Location {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::LidarMeasurement &self, size_t pos, const cr::Location &point) {
      self.at(pos) = point;
    })
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::CollisionEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::CollisionEvent>>("CollisionEvent", no_init)
    .add_property("actor", &csd::CollisionEvent::GetActor)
    .add_property("other_actor", &csd::CollisionEvent::GetOtherActor)
    .add_property("normal_impulse", CALL_RETURNING_COPY(csd::CollisionEvent, GetNormalImpulse))
    .def(self_ns::str(self_ns::self))
  ;
}
