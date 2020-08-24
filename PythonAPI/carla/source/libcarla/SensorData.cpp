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
#include <carla/sensor/data/SemanticLidarMeasurement.h>
#include <carla/sensor/data/GnssMeasurement.h>
#include <carla/sensor/data/RadarMeasurement.h>
#include <carla/sensor/data/DVSEventArray.h>

#include <carla/sensor/data/RadarData.h>

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

  std::ostream &operator<<(std::ostream &out, const SemanticLidarMeasurement &meas) {
    out << "SemanticLidarMeasurement(frame=" << std::to_string(meas.GetFrame())
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

  std::ostream &operator<<(std::ostream &out, const DVSEvent &event) {
    out << "Event(x=" << std::to_string(event.x)
        << ", y=" << std::to_string(event.y)
        << ", t=" << std::to_string(event.t)
        << ", pol=" << std::to_string(event.pol) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const DVSEventArray &events) {
    out << "EventArray(frame=" << std::to_string(events.GetFrame())
        << ", timestamp=" << std::to_string(events.GetTimestamp())
        << ", dimensions=" << std::to_string(events.GetWidth()) << 'x' << std::to_string(events.GetHeight())
        << ", number_of_events=" << std::to_string(events.size())
        << ')';
    return out;
  }


  std::ostream &operator<<(std::ostream &out, const RadarDetection &det) {
    out << "RadarDetection(velocity=" << std::to_string(det.velocity)
        << ", azimuth=" << std::to_string(det.azimuth)
        << ", altitude=" << std::to_string(det.altitude)
        << ", depth=" << std::to_string(det.depth)
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const LidarDetection &det) {
    out << "LidarDetection(x=" << std::to_string(det.point.x)
        << ", y=" << std::to_string(det.point.y)
        << ", z=" << std::to_string(det.point.z)
        << ", intensity=" << std::to_string(det.intensity)
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const SemanticLidarDetection &det) {
    out << "SemanticLidarDetection(x=" << std::to_string(det.point.x)
        << ", y=" << std::to_string(det.point.y)
        << ", z=" << std::to_string(det.point.z)
        << ", cos_inc_angle=" << std::to_string(det.cos_inc_angle)
        << ", object_idx=" << std::to_string(det.object_idx)
        << ", object_tag=" << std::to_string(det.object_tag)
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
    .add_property("frame", &cs::SensorData::GetFrame)
    .add_property("frame_number", &cs::SensorData::GetFrame) // deprecated.
    .add_property("timestamp", &cs::SensorData::GetTimestamp)
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
    .def("__getitem__", +[](const csd::LidarMeasurement &self, size_t pos) -> csd::LidarDetection {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::LidarMeasurement &self, size_t pos, const csd::LidarDetection &detection) {
      self.at(pos) = detection;
    })
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::SemanticLidarMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::SemanticLidarMeasurement>>("SemanticLidarMeasurement", no_init)
    .add_property("horizontal_angle", &csd::SemanticLidarMeasurement::GetHorizontalAngle)
    .add_property("channels", &csd::SemanticLidarMeasurement::GetChannelCount)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::SemanticLidarMeasurement>)
    .def("get_point_count", &csd::SemanticLidarMeasurement::GetPointCount, (arg("channel")))
    .def("save_to_disk", &SavePointCloudToDisk<csd::SemanticLidarMeasurement>, (arg("path")))
    .def("__len__", &csd::SemanticLidarMeasurement::size)
    .def("__iter__", iterator<csd::SemanticLidarMeasurement>())
    .def("__getitem__", +[](const csd::SemanticLidarMeasurement &self, size_t pos) -> csd::SemanticLidarDetection {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::SemanticLidarMeasurement &self, size_t pos, const csd::SemanticLidarDetection &detection) {
      self.at(pos) = detection;
    })
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::CollisionEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::CollisionEvent>>("CollisionEvent", no_init)
    .add_property("actor", &csd::CollisionEvent::GetActor)
    .add_property("other_actor", &csd::CollisionEvent::GetOtherActor)
    .add_property("normal_impulse", CALL_RETURNING_COPY(csd::CollisionEvent, GetNormalImpulse))
    .def(self_ns::str(self_ns::self))
  ;

    class_<csd::ObstacleDetectionEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::ObstacleDetectionEvent>>("ObstacleDetectionEvent", no_init)
    .add_property("actor", &csd::ObstacleDetectionEvent::GetActor)
    .add_property("other_actor", &csd::ObstacleDetectionEvent::GetOtherActor)
    .add_property("distance", CALL_RETURNING_COPY(csd::ObstacleDetectionEvent, GetDistance))
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::LaneInvasionEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::LaneInvasionEvent>>("LaneInvasionEvent", no_init)
    .add_property("actor", &csd::LaneInvasionEvent::GetActor)
    .add_property("crossed_lane_markings", CALL_RETURNING_LIST(csd::LaneInvasionEvent, GetCrossedLaneMarkings))
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::GnssMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::GnssMeasurement>>("GnssMeasurement", no_init)
    .add_property("latitude", &csd::GnssMeasurement::GetLatitude)
    .add_property("longitude", &csd::GnssMeasurement::GetLongitude)
    .add_property("altitude", &csd::GnssMeasurement::GetAltitude)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::IMUMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::IMUMeasurement>>("IMUMeasurement", no_init)
    .add_property("accelerometer", &csd::IMUMeasurement::GetAccelerometer)
    .add_property("gyroscope", &csd::IMUMeasurement::GetGyroscope)
    .add_property("compass", &csd::IMUMeasurement::GetCompass)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::RadarMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::RadarMeasurement>>("RadarMeasurement", no_init)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::RadarMeasurement>)
    .def("get_detection_count", &csd::RadarMeasurement::GetDetectionAmount)
    .def("__len__", &csd::RadarMeasurement::size)
    .def("__iter__", iterator<csd::RadarMeasurement>())
    .def("__getitem__", +[](const csd::RadarMeasurement &self, size_t pos) -> csd::RadarDetection {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::RadarMeasurement &self, size_t pos, const csd::RadarDetection &detection) {
      self.at(pos) = detection;
    })
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::RadarDetection>("RadarDetection")
    .def_readwrite("velocity", &csd::RadarDetection::velocity)
    .def_readwrite("azimuth", &csd::RadarDetection::azimuth)
    .def_readwrite("altitude", &csd::RadarDetection::altitude)
    .def_readwrite("depth", &csd::RadarDetection::depth)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::LidarDetection>("LidarDetection")
    .def_readwrite("point", &csd::LidarDetection::point)
    .def_readwrite("intensity", &csd::LidarDetection::intensity)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::SemanticLidarDetection>("SemanticLidarDetection")
    .def_readwrite("point", &csd::SemanticLidarDetection::point)
    .def_readwrite("cos_inc_angle", &csd::SemanticLidarDetection::cos_inc_angle)
    .def_readwrite("object_idx", &csd::SemanticLidarDetection::object_idx)
    .def_readwrite("object_tag", &csd::SemanticLidarDetection::object_tag)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::DVSEvent>("DVSEvent")
    .add_property("x", &csd::DVSEvent::x)
    .add_property("y", &csd::DVSEvent::y)
    .add_property("t", &csd::DVSEvent::t)
    .add_property("pol", &csd::DVSEvent::pol)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::DVSEventArray, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::DVSEventArray>>("DVSEventArray", no_init)
    .add_property("width", &csd::DVSEventArray::GetWidth)
    .add_property("height", &csd::DVSEventArray::GetHeight)
    .add_property("fov", &csd::DVSEventArray::GetFOVAngle)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::DVSEventArray>)
    .def("__len__", &csd::DVSEventArray::size)
    .def("__iter__", iterator<csd::DVSEventArray>())
    .def("__getitem__", +[](const csd::DVSEventArray &self, size_t pos) -> csd::DVSEvent {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::DVSEventArray &self, size_t pos, csd::DVSEvent event) {
      self.at(pos) = event;
    })
    .def("to_image", CALL_RETURNING_LIST(csd::DVSEventArray, ToImage))
    .def("to_array", CALL_RETURNING_LIST(csd::DVSEventArray, ToArray))
    .def("to_array_x", CALL_RETURNING_LIST(csd::DVSEventArray, ToArrayX))
    .def("to_array_y", CALL_RETURNING_LIST(csd::DVSEventArray, ToArrayY))
    .def("to_array_t", CALL_RETURNING_LIST(csd::DVSEventArray, ToArrayT))
    .def("to_array_pol", CALL_RETURNING_LIST(csd::DVSEventArray, ToArrayPol))
    .def(self_ns::str(self_ns::self))
  ;
}
