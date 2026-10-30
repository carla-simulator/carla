// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/sensor/SensorRegistry.h>
#include <carla/sensor/s11n/SensorHeaderSerializer.h>
#include <cstring>
#include <memory>
#include <type_traits>

namespace {

  static std::weak_ptr<void> ValidationNativeOwner;
  static bool ValidationNativeAlive() { return !ValidationNativeOwner.expired(); }
  static long ValidationNativeCount() { return ValidationNativeOwner.use_count(); }

  template <typename T>
  static carla::SharedPtr<T> ValidationDecode(size_t type, const carla::Buffer &payload) {
    const auto header = carla::sensor::s11n::SensorHeaderSerializer::Serialize(type, 42u, 0.5, {});
    carla::Buffer message;
    message.copy_from(carla::sensor::s11n::SensorHeaderSerializer::header_offset, payload);
    std::memcpy(message.data(), header.data(), header.size());
    return std::dynamic_pointer_cast<T>(carla::sensor::SensorRegistry::Deserialize(std::move(message)));
  }

  static carla::SharedPtr<carla::sensor::data::RadarMeasurement> ValidationRadar(float seed, uint32_t count) {
    carla::sensor::data::RadarData detections;
    for (size_t index = 0; index < count; ++index) {
      detections.WriteDetection({0.f, 0.f, 0.f, seed + static_cast<float>(index)});
    }
    const auto payload = carla::sensor::s11n::RadarSerializer::Serialize(0, detections, carla::Buffer{});
    auto result = ValidationDecode<carla::sensor::data::RadarMeasurement>(
        carla::sensor::SensorRegistry::get<ARadar *>::index, payload);
    ValidationNativeOwner = result;
    return result;
  }

  struct ValidationCameraDescription {
    uint32_t GetImageWidth() const { return 3u; }
    uint32_t GetImageHeight() const { return 1u; }
    float GetFOVAngle() const { return 90.f; }
  };

  static carla::SharedPtr<carla::sensor::data::Image> ValidationImage() {
    using Serializer = carla::sensor::s11n::ImageSerializer;
    std::vector<carla::sensor::data::Color> pixels = {{1u, 2u, 3u}, {4u, 5u, 6u}, {7u, 8u, 9u}};
    carla::Buffer bitmap;
    bitmap.copy_from(Serializer::header_offset, pixels);
    const auto payload = Serializer::Serialize(ValidationCameraDescription{}, std::move(bitmap));
    auto result = ValidationDecode<carla::sensor::data::Image>(
        carla::sensor::SensorRegistry::get<ASceneCaptureCamera *>::index, payload);
    ValidationNativeOwner = result;
    return result;
  }

  static carla::SharedPtr<carla::sensor::data::OpticalFlowImage> ValidationFlow() {
    using Serializer = carla::sensor::s11n::OpticalFlowImageSerializer;
    std::vector<carla::sensor::data::OpticalFlowPixel> pixels = {{0.01f, 0.0f}, {0.0f, 0.01f}, {-0.01f, 0.0f}};
    carla::Buffer bitmap;
    bitmap.copy_from(Serializer::header_offset, pixels);
    const auto payload = Serializer::Serialize(ValidationCameraDescription{}, std::move(bitmap));
    auto result = ValidationDecode<carla::sensor::data::OpticalFlowImage>(
        carla::sensor::SensorRegistry::get<AOpticalFlowCamera *>::index, payload);
    ValidationNativeOwner = result;
    return result;
  }

  template <typename T>
  static bool ValidationTrackShared(T &object) {
    if constexpr (std::is_base_of_v<carla::EnableSharedFromThis<T>, T>) {
      ValidationNativeOwner = object.shared_from_this();
      return true;
    } else {
      ValidationNativeOwner.reset();
      return false;
    }
  }

  static bool ValidationTrackFake(FakeImage &object) { return ValidationTrackShared(object); }

  void RegisterSensorDataTestFactories() {
    using namespace boost::python;
    def("_validation_radar", &ValidationRadar, (arg("seed"), arg("count")=8u));
    def("_validation_image", &ValidationImage);
    def("_validation_flow", &ValidationFlow);
    def("_validation_track_fake", &ValidationTrackFake);
    def("_validation_native_alive", &ValidationNativeAlive);
    def("_validation_native_count", &ValidationNativeCount);
  }

} // namespace
