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
#include <carla/sensor/data/IMUMeasurement.h>
#include <carla/sensor/data/ObstacleDetectionEvent.h>
#include <carla/sensor/data/Image.h>
#include <carla/sensor/data/LaneInvasionEvent.h>
#include <carla/sensor/data/LidarMeasurement.h>
#include <carla/sensor/data/GnssMeasurement.h>
#include <carla/sensor/data/RadarMeasurement.h>

#include <carla/sensor/s11n/RadarData.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <ostream>
#include <iostream>

namespace carla {
namespace sensor {
namespace data {

  std::ostream &operator<<(std::ostream &out, const Image &image) {
    out << "Image(frame=" << std::to_string(image.GetFrame())
        << ", timestamp=" << std::to_string(image.GetTimestamp())
        << ", size=" << std::to_string(image.GetWidth()) << 'x' << std::to_string(image.GetHeight())
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const LidarMeasurement &meas) {
    out << "LidarMeasurement(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", number_of_points=" << std::to_string(meas.size())
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const CollisionEvent &meas) {
    out << "CollisionEvent(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", other_actor=" << meas.GetOtherActor()
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const ObstacleDetectionEvent &meas) {
    out << "ObstacleDetectionEvent(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", other_actor=" << meas.GetOtherActor()
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const LaneInvasionEvent &meas) {
    out << "LaneInvasionEvent(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const GnssMeasurement &meas) {
    out << "GnssMeasurement(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", lat=" << std::to_string(meas.GetLatitude())
        << ", lon=" << std::to_string(meas.GetLongitude())
        << ", alt=" << std::to_string(meas.GetAltitude())
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const IMUMeasurement &meas) {
    out << "IMUMeasurement(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", accelerometer=" << meas.GetAccelerometer()
        << ", gyroscope=" << meas.GetGyroscope()
        << ", compass=" << std::to_string(meas.GetCompass())
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const RadarMeasurement &meas) {
    out << "RadarMeasurement(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", point_count=" << std::to_string(meas.GetDetectionAmount())
        << ')';
    return out;
  }

} // namespace data

namespace s11n {

  std::ostream &operator<<(std::ostream &out, const RadarDetection &det) {
    out << "RadarDetection(velocity=" << std::to_string(det.velocity)
        << ", azimuth=" << std::to_string(det.azimuth)
        << ", altitude=" << std::to_string(det.altitude)
        << ", depth=" << std::to_string(det.depth)
        << ')';
    return out;
  }

} // namespace s11n
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
  auto size = static_cast<Py_ssize_t>(sizeof(typename T::value_type) * self.size());
#if PY_MAJOR_VERSION >= 3
  auto *ptr = PyMemoryView_FromMemory(reinterpret_cast<char *>(data), size, PyBUF_READ);
#else
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
  namespace css = carla::sensor::s11n;

  class_<cs::SensorData, boost::noncopyable, boost::shared_ptr<cs::SensorData>>("SensorData", no_init)
    .add_property("frame", &cs::SensorData::GetFrame, "@DocString(SensorData.frame)")
    .add_property("frame_number", &cs::SensorData::GetFrame, "@DocString(SensorData.frame_number)") // deprecated.
    .add_property("timestamp", &cs::SensorData::GetTimestamp, "@DocString(SensorData.timestamp)")
    .add_property("transform", CALL_RETURNING_COPY(cs::SensorData, GetSensorTransform), "@DocString(SensorData.transform)")
  ;

  enum_<EColorConverter>("ColorConverter")
    .value("Raw", EColorConverter::Raw)
    .value("Depth", EColorConverter::Depth)
    .value("LogarithmicDepth", EColorConverter::LogarithmicDepth)
    .value("CityScapesPalette", EColorConverter::CityScapesPalette)
  ;

  class_<csd::Image, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::Image>>("Image", no_init)
    .add_property("width", &csd::Image::GetWidth, "@DocString(Image.width)")
    .add_property("height", &csd::Image::GetHeight, "@DocString(Image.height)")
    .add_property("fov", &csd::Image::GetFOVAngle, "@DocString(Image.fov)")
    .add_property("raw_data", &GetRawDataAsBuffer<csd::Image>, "@DocString(Image.raw_data)")
    .def("convert", &ConvertImage<csd::Image>, (arg("color_converter")), "@DocString(Image.convert)")
    .def("save_to_disk", &SaveImageToDisk<csd::Image>, (arg("path"), arg("color_converter")=EColorConverter::Raw), "@DocString(Image.save_to_disk)")
    .def("__len__", &csd::Image::size, "@DocString(Image.__len__)")
    .def("__iter__", iterator<csd::Image>(), "@DocString(Image.__iter__)")
    .def("__getitem__", +[](const csd::Image &self, size_t pos) -> csd::Color {
      return self.at(pos);
    }, "@DocString(Image.__getitem__)")
    .def("__setitem__", +[](csd::Image &self, size_t pos, csd::Color color) {
      self.at(pos) = color;
    }, "@DocString(Image.__setitem__)")
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::LidarMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::LidarMeasurement>>("LidarMeasurement", no_init)
    .add_property("horizontal_angle", &csd::LidarMeasurement::GetHorizontalAngle, "@DocString(LidarMeasurement.horizontal_angle)")
    .add_property("channels", &csd::LidarMeasurement::GetChannelCount, "@DocString(LidarMeasurement.channels)")
    .add_property("raw_data", &GetRawDataAsBuffer<csd::LidarMeasurement>, "@DocString(LidarMeasurement.raw_data)")
    .def("get_point_count", &csd::LidarMeasurement::GetPointCount, (arg("channel")), "@DocString(LidarMeasurement.get_point_count)")
    .def("save_to_disk", &SavePointCloudToDisk<csd::LidarMeasurement>, (arg("path")), "@DocString(LidarMeasurement.save_to_disk)")
    .def("__len__", &csd::LidarMeasurement::size, "@DocString(LidarMeasurement.__len__)")
    .def("__iter__", iterator<csd::LidarMeasurement>(), "@DocString(LidarMeasurement.__iter__)")
    .def("__getitem__", +[](const csd::LidarMeasurement &self, size_t pos) -> cr::Location {
      return self.at(pos);
    }, "@DocString(LidarMeasurement.__getitem__)")
    .def("__setitem__", +[](csd::LidarMeasurement &self, size_t pos, const cr::Location &point) {
      self.at(pos) = point;
    }, "@DocString(LidarMeasurement.__setitem__)")
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::CollisionEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::CollisionEvent>>("CollisionEvent", no_init)
    .add_property("actor", &csd::CollisionEvent::GetActor, "@DocString(CollisionEvent.actor)")
    .add_property("other_actor", &csd::CollisionEvent::GetOtherActor, "@DocString(CollisionEvent.other_actor)")
    .add_property("normal_impulse", CALL_RETURNING_COPY(csd::CollisionEvent, GetNormalImpulse), "@DocString(CollisionEvent.normal_impulse)")
    .def(self_ns::str(self_ns::self))
  ;

    class_<csd::ObstacleDetectionEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::ObstacleDetectionEvent>>("ObstacleDetectionEvent", no_init)
    .add_property("actor", &csd::ObstacleDetectionEvent::GetActor, "@DocString(ObstacleDetectionEvent.actor)")
    .add_property("other_actor", &csd::ObstacleDetectionEvent::GetOtherActor, "@DocString(ObstacleDetectionEvent.other_actor)")
    .add_property("distance", CALL_RETURNING_COPY(csd::ObstacleDetectionEvent, GetDistance), "@DocString(ObstacleDetectionEvent.distance)")
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::LaneInvasionEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::LaneInvasionEvent>>("LaneInvasionEvent", no_init)
    .add_property("actor", &csd::LaneInvasionEvent::GetActor, "@DocString(LaneInvasionEvent.actor)")
    .add_property("crossed_lane_markings", CALL_RETURNING_LIST(csd::LaneInvasionEvent, GetCrossedLaneMarkings), "@DocString(LaneInvasionEvent.crossed_lane_markings)")
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::GnssMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::GnssMeasurement>>("GnssMeasurement", no_init)
    .add_property("latitude", &csd::GnssMeasurement::GetLatitude, "@DocString(GnssMeasurement.latitude)")
    .add_property("longitude", &csd::GnssMeasurement::GetLongitude, "@DocString(GnssMeasurement.longitude)")
    .add_property("altitude", &csd::GnssMeasurement::GetAltitude, "@DocString(GnssMeasurement.altitude)")
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::IMUMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::IMUMeasurement>>("IMUMeasurement", no_init)
    .add_property("accelerometer", &csd::IMUMeasurement::GetAccelerometer, "@DocString(IMUMeasurement.accelerometer)")
    .add_property("gyroscope", &csd::IMUMeasurement::GetGyroscope, "@DocString(IMUMeasurement.gyroscope)")
    .add_property("compass", &csd::IMUMeasurement::GetCompass, "@DocString(IMUMeasurement.compass)")
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::RadarMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::RadarMeasurement>>("RadarMeasurement", no_init)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::RadarMeasurement>, "@DocString(RadarMeasurement.raw_data)")
    .def("get_detection_count", &csd::RadarMeasurement::GetDetectionAmount, "@DocString(RadarMeasurement.get_detection_count)")
    .def("__len__", &csd::RadarMeasurement::size, "@DocString(RadarMeasurement.__len__)")
    .def("__iter__", iterator<csd::RadarMeasurement>(), "@DocString(RadarMeasurement.__iter__)")
    .def("__getitem__", +[](const csd::RadarMeasurement &self, size_t pos) -> css::RadarDetection {
      return self.at(pos);
    }, "@DocString(RadarMeasurement.__getitem__)")
    .def("__setitem__", +[](csd::RadarMeasurement &self, size_t pos, const css::RadarDetection &detection) {
      self.at(pos) = detection;
    }, "@DocString(RadarMeasurement.__setitem__)")
    .def(self_ns::str(self_ns::self))
  ;

  class_<css::RadarDetection>("RadarDetection")
    .def_readwrite("velocity", &css::RadarDetection::velocity, "@DocString(RadarDetection.velocity)")
    .def_readwrite("azimuth", &css::RadarDetection::azimuth, "@DocString(RadarDetection.azimuth)")
    .def_readwrite("altitude", &css::RadarDetection::altitude, "@DocString(RadarDetection.altitude)")
    .def_readwrite("depth", &css::RadarDetection::depth, "@DocString(RadarDetection.depth)")
    .def(self_ns::str(self_ns::self))
  ;
}
