// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PythonAPI.h>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

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

  std::ostream &operator<<(std::ostream &out, const OpticalFlowImage &image) {
    out << "OpticalFlowImage(frame=" << std::to_string(image.GetFrame())
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

// image object resturned from optical flow to color conversion
class FakeImage : public std::vector<uint8_t> {
  public:
  unsigned int Width = 0;
  unsigned int Height = 0;
  float FOV = 0;
};
// method to convert optical flow images to rgb
static FakeImage ColorCodedFlow (
    carla::sensor::data::OpticalFlowImage& image) {
  namespace bp = boost::python;
  namespace csd = carla::sensor::data;
  constexpr float pi = 3.1415f;
  constexpr float rad2ang = 360.f/(2.f*pi);
  FakeImage result;
  result.Width = image.GetWidth();
  result.Height = image.GetHeight();
  result.FOV = image.GetFOVAngle();
  result.resize(image.GetHeight()*image.GetWidth()* 4);

  // lambda for computing batches of pixels
  auto command = [&] (size_t min_index, size_t max_index) {
    for (size_t index = min_index; index < max_index; index++) {
      const csd::OpticalFlowPixel& pixel = image[index];
      float vx = pixel.x;
      float vy = pixel.y;

      float angle = 180.f + std::atan2(vy, vx)*rad2ang;
      if (angle < 0) angle = 360.f + angle;
      angle = std::fmod(angle, 360.f);

      float norm = std::sqrt(vx*vx + vy*vy);
      const float shift = 0.999f;
      const float a = 1.f/std::log(0.1f + shift);
      float intensity = carla::geom::Math::Clamp(a*std::log(norm + shift), 0.f, 1.f);

      float& H = angle;
      float  S = 1.f;
      float V = intensity;
      float H_60 = H*(1.f/60.f);

      float C = V * S;
      float X = C*(1.f - std::abs(std::fmod(H_60, 2.f) - 1.f));
      float m = V - C;

      float r = 0,g = 0,b = 0;
      unsigned int angle_case = static_cast<unsigned int>(H_60);
      switch (angle_case) {
      case 0:
        r = C;
        g = X;
        b = 0;
        break;
      case 1:
        r = X;
        g = C;
        b = 0;
        break;
      case 2:
        r = 0;
        g = C;
        b = X;
        break;
      case 3:
        r = 0;
        g = X;
        b = C;
        break;
      case 4:
        r = X;
        g = 0;
        b = C;
        break;
      case 5:
        r = C;
        g = 0;
        b = X;
        break;
      default:
        r = 1;
        g = 1;
        b = 1;
        break;
      }

      uint8_t R = static_cast<uint8_t>((r+m)*255.f);
      uint8_t G = static_cast<uint8_t>((g+m)*255.f);
      uint8_t B = static_cast<uint8_t>((b+m)*255.f);
      result[4*index] = B;
      result[4*index + 1] = G;
      result[4*index + 2] = R;
      result[4*index + 3] = 0;
    }
  };
  size_t num_threads = std::max(8u, std::thread::hardware_concurrency());
  size_t batch_size = image.size() / num_threads;
  std::vector<std::thread*> t(num_threads+1);

  for(size_t n = 0; n < num_threads; n++) {
    t[n] = new std::thread(command, n * batch_size, (n+1) * batch_size);
  }
  t[num_threads] = new std::thread(command, num_threads * batch_size, image.size());

  for(size_t n = 0; n <= num_threads; n++) {
    if(t[n]->joinable()){
      t[n]->join();
    }
    delete t[n];
  }
  return result;
}

template <typename T>
static std::string SaveImageToDisk(T &self, std::string path, EColorConverter cc) {
  std::string r;
  carla::PythonUtil::ReleaseGIL unlock;
  using namespace carla::image;
  auto view = ImageView::MakeView(self);
  switch (cc) {
    case EColorConverter::Raw:
      r = ImageIO::WriteView(
          std::move(path),
          view);
      break;
    case EColorConverter::Depth:
      r = ImageIO::WriteView(
          std::move(path),
          ImageView::MakeColorConvertedView(view, ColorConverter::Depth()));
      break;
    case EColorConverter::LogarithmicDepth:
      r = ImageIO::WriteView(
          std::move(path),
          ImageView::MakeColorConvertedView(view, ColorConverter::LogarithmicDepth()));
      break;
    case EColorConverter::CityScapesPalette:
      r = ImageIO::WriteView(
          std::move(path),
          ImageView::MakeColorConvertedView(view, ColorConverter::CityScapesPalette()));
      break;
    default:
      throw std::invalid_argument("invalid color converter!");
  }
  return r;
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

  // Fake image returned from optical flow to color conversion
  // fakes the regular image object. Only used for visual purposes
  class_<FakeImage>("FakeImage", no_init)
      .def(vector_indexing_suite<std::vector<uint8_t>>())
      .add_property("width", &FakeImage::Width)
      .add_property("height", &FakeImage::Height)
      .add_property("fov", &FakeImage::FOV)
      .add_property("raw_data", &GetRawDataAsBuffer<FakeImage>);

  class_<cs::SensorData, boost::noncopyable, std::shared_ptr<cs::SensorData>>("SensorData", no_init)
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

  // The values here should match the ones in the enum EGBufferTextureID,
  // from the CARLA fork of Unreal Engine (Renderer/Public/GBufferView.h).
  enum_<int>("GBufferTextureID")
    .value("SceneColor", 0)
    .value("SceneDepth", 1)
    .value("SceneStencil", 2)
    .value("GBufferA", 3)
    .value("GBufferB", 4)
    .value("GBufferC", 5)
    .value("GBufferD", 6)
    .value("GBufferE", 7)
    .value("GBufferF", 8)
    .value("Velocity", 9)
    .value("SSAO", 10)
    .value("CustomDepth", 11)
    .value("CustomStencil", 12)
  ;

  class_<csd::Image, bases<cs::SensorData>, boost::noncopyable, std::shared_ptr<csd::Image>>("Image", no_init)
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

  class_<csd::OpticalFlowImage, bases<cs::SensorData>, boost::noncopyable, std::shared_ptr<csd::OpticalFlowImage>>("OpticalFlowImage", no_init)
    .add_property("width", &csd::OpticalFlowImage::GetWidth)
    .add_property("height", &csd::OpticalFlowImage::GetHeight)
    .add_property("fov", &csd::OpticalFlowImage::GetFOVAngle)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::OpticalFlowImage>)
    .def("get_color_coded_flow", &ColorCodedFlow)
    .def("__len__", &csd::OpticalFlowImage::size)
    .def("__iter__", iterator<csd::OpticalFlowImage>())
    .def("__getitem__", +[](const csd::OpticalFlowImage &self, size_t pos) -> csd::OpticalFlowPixel {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::OpticalFlowImage &self, size_t pos, csd::OpticalFlowPixel color) {
      self.at(pos) = color;
    })
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::LidarMeasurement, bases<cs::SensorData>, boost::noncopyable, std::shared_ptr<csd::LidarMeasurement>>("LidarMeasurement", no_init)
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

  class_<csd::SemanticLidarMeasurement, bases<cs::SensorData>, boost::noncopyable, std::shared_ptr<csd::SemanticLidarMeasurement>>("SemanticLidarMeasurement", no_init)
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

  class_<csd::CollisionEvent, bases<cs::SensorData>, boost::noncopyable, std::shared_ptr<csd::CollisionEvent>>("CollisionEvent", no_init)
    .add_property("actor", &csd::CollisionEvent::GetActor)
    .add_property("other_actor", &csd::CollisionEvent::GetOtherActor)
    .add_property("normal_impulse", CALL_RETURNING_COPY(csd::CollisionEvent, GetNormalImpulse))
    .def(self_ns::str(self_ns::self))
  ;

    class_<csd::ObstacleDetectionEvent, bases<cs::SensorData>, boost::noncopyable, std::shared_ptr<csd::ObstacleDetectionEvent>>("ObstacleDetectionEvent", no_init)
    .add_property("actor", &csd::ObstacleDetectionEvent::GetActor)
    .add_property("other_actor", &csd::ObstacleDetectionEvent::GetOtherActor)
    .add_property("distance", CALL_RETURNING_COPY(csd::ObstacleDetectionEvent, GetDistance))
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::LaneInvasionEvent, bases<cs::SensorData>, boost::noncopyable, std::shared_ptr<csd::LaneInvasionEvent>>("LaneInvasionEvent", no_init)
    .add_property("actor", &csd::LaneInvasionEvent::GetActor)
    .add_property("crossed_lane_markings", CALL_RETURNING_LIST(csd::LaneInvasionEvent, GetCrossedLaneMarkings))
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::GnssMeasurement, bases<cs::SensorData>, boost::noncopyable, std::shared_ptr<csd::GnssMeasurement>>("GnssMeasurement", no_init)
    .add_property("latitude", &csd::GnssMeasurement::GetLatitude)
    .add_property("longitude", &csd::GnssMeasurement::GetLongitude)
    .add_property("altitude", &csd::GnssMeasurement::GetAltitude)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::IMUMeasurement, bases<cs::SensorData>, boost::noncopyable, std::shared_ptr<csd::IMUMeasurement>>("IMUMeasurement", no_init)
    .add_property("accelerometer", &csd::IMUMeasurement::GetAccelerometer)
    .add_property("gyroscope", &csd::IMUMeasurement::GetGyroscope)
    .add_property("compass", &csd::IMUMeasurement::GetCompass)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::RadarMeasurement, bases<cs::SensorData>, boost::noncopyable, std::shared_ptr<csd::RadarMeasurement>>("RadarMeasurement", no_init)
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

  class_<csd::DVSEventArray, bases<cs::SensorData>, boost::noncopyable, std::shared_ptr<csd::DVSEventArray>>("DVSEventArray", no_init)
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
