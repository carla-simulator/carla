// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Logging.h"
#include "carla/ros2/ROS2.h"
#include "carla/geom/GeoLocation.h"
#include "carla/geom/Vector3D.h"
#include "carla/sensor/data/DVSEvent.h"
#include "carla/sensor/data/LidarData.h"
#include "carla/sensor/data/SemanticLidarData.h"
#include "carla/sensor/data/RadarData.h"
#include "carla/sensor/data/Image.h"
#include "carla/sensor/s11n/ImageSerializer.h"
#include "carla/sensor/s11n/SensorHeaderSerializer.h"

#include "publishers/BasePublisher.h"
#include "publishers/CarlaCameraPublisher.h"
#include "publishers/CarlaClockPublisher.h"
#include "publishers/CarlaRGBCameraPublisher.h"
#include "publishers/CarlaDepthCameraPublisher.h"
#include "publishers/CarlaNormalsCameraPublisher.h"
#include "publishers/CarlaOpticalFlowCameraPublisher.h"
#include "publishers/CarlaSSCameraPublisher.h"
#include "publishers/CarlaISCameraPublisher.h"
#include "publishers/CarlaDVSCameraPublisher.h"
#include "publishers/CarlaLidarPublisher.h"
#include "publishers/CarlaSemanticLidarPublisher.h"
#include "publishers/CarlaRadarPublisher.h"
#include "publishers/CarlaIMUPublisher.h"
#include "publishers/CarlaGNSSPublisher.h"
#include "publishers/CarlaTransformPublisher.h"
#include "publishers/CarlaCollisionPublisher.h"
#include "publishers/BasicPublisher.h"

#include "subscribers/AckermannControlSubscriber.h"
#include "subscribers/BaseSubscriber.h"
#include "subscribers/CarlaEgoVehicleControlSubscriber.h"
#include "subscribers/CarlaSubscriber.h"
#if defined(WITH_ROS2_DEMO)
  #include "subscribers/BasicSubscriber.h"
#endif

#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace carla {
namespace ros2 {

// static fields
std::shared_ptr<ROS2> ROS2::_instance;

// list of sensors (should be equal to the list of SensorsRegistry)
enum ESensors {
  CollisionSensor,
  DepthCamera,
  NormalsCamera,
  DVSCamera,
  GnssSensor,
  InertialMeasurementUnit,
  LaneInvasionSensor,
  ObstacleDetectionSensor,
  OpticalFlowCamera,
  Radar,
  RayCastSemanticLidar,
  RayCastLidar,
  RssSensor,
  SceneCaptureCamera,
  SemanticSegmentationCamera,
  InstanceSegmentationCamera,
  WorldObserver,
  // Keep these in lock-step with the SensorRegistry tuple order: the fisheye /
  // wide-angle-lens cameras (ported in #9741) were added to SensorRegistry but
  // were missing here, which shifted every following value out of sync with the
  // registry index.
  SceneCaptureCamera_WideAngleLens,
  DepthCamera_WideAngleLens,
  InstanceSegmentationCamera_WideAngleLens,
  SemanticSegmentationCamera_WideAngleLens,
  CameraGBufferUint8,
  CameraGBufferFloat,
  HSSLidar
};

void ROS2::Enable(bool enable) {
  _enabled = enable;
  log_info("ROS2 enabled: ", _enabled);
  _clock_publisher = std::make_shared<CarlaClockPublisher>();
#if defined(WITH_ROS2_DEMO)
  _basic_publisher = std::make_shared<BasicPublisher>("basic_publisher", "");
  _basic_publisher->Init();
#endif
}

void ROS2::SetFrame(uint64_t frame) {
  _frame = frame;
  for (auto &element : _subscribers) {
    void *actor = element.first;
    auto &subscriber = element.second;
    auto callback_it = _actor_callbacks.find(actor);
    if (callback_it != _actor_callbacks.end()) {
      subscriber->ProcessMessages(callback_it->second);
    }
  }
#if defined(WITH_ROS2_DEMO)
  if (_basic_subscriber) {
    void *actor = _basic_subscriber->GetActor();
    if (!_basic_subscriber->IsAlive()) {
      RemoveBasicSubscriberCallback(actor);
    }
    if (actor && _basic_subscriber->HasNewMessage()) {
      auto it = _actor_message_callbacks.find(actor);
      if (it != _actor_message_callbacks.end()) {
        MessageControl control;
        control.message = _basic_subscriber->GetMessage();
        it->second(actor, control);
      }
    }
  }
#endif
}

void ROS2::SetTimestamp(double timestamp) {
  double integral;
  const double fractional = std::modf(timestamp, &integral);
  const double multiplier = 1000000000.0;
  _seconds = static_cast<int32_t>(integral);
  _nanoseconds = static_cast<uint32_t>(fractional * multiplier);
  if (_clock_publisher) {
    _clock_publisher->Write(_seconds, _nanoseconds);
    _clock_publisher->Publish();
  }
#if defined(WITH_ROS2_DEMO)
  _basic_publisher->SetData("Hello from Carla!");
  _basic_publisher->Publish();
#endif
}

void ROS2::RegisterSensor(
    void *actor, std::string ros_name, std::string frame_id, bool publish_tf) {
  // insert_or_assign so re-registering an actor with a new ros_name actually
  // updates the entry; unordered_map::insert would silently keep the stale
  // one.
  _registrations.insert_or_assign(
      actor, ActorRegistration{std::move(ros_name), std::move(frame_id), publish_tf});
}

void ROS2::UnregisterSensor(void *actor) {
  _publishers.erase(actor);
  _camera_publishers.erase(actor);
  _transforms.erase(actor);
  _actor_parents.erase(actor);
  _registrations.erase(actor);
}

void ROS2::RegisterVehicle(
    void *actor, std::string ros_name, std::string frame_id, ActorCallback callback,
    bool enable_ackermann_control) {
  _registrations.insert_or_assign(
      actor, ActorRegistration{ros_name, frame_id, true});

  // Idempotency: drop any prior subscribers / callbacks bound to this actor
  // so a re-registration does not accumulate duplicate DataReaders nor leave
  // the previous callback wired.
  _subscribers.erase(actor);
  _actor_callbacks.insert_or_assign(actor, std::move(callback));

  // The legacy CarlaEgoVehicleControlSubscriber::Init built its topic as
  // "rt/carla/" + [parent + "/"] + name + "/vehicle_control_cmd". With the
  // new template constructors the suffix is appended inside each subscriber,
  // so we hand them the base path only.
  const std::string base_topic_name = "rt/carla/" + ros_name;

  // The two control modes are mutually exclusive: a vehicle listens on either the
  // Ackermann topic or the direct VehicleControl topic, never both, so an Ackermann
  // message can never latch ApplyVehicleAckermannControl while plain VehicleControl
  // messages keep arriving on the other topic. Ackermann is opt-in per youtalk's review.
  if (enable_ackermann_control) {
    _subscribers.insert({actor, std::make_shared<AckermannControlSubscriber>(actor, base_topic_name, std::move(frame_id))});
  } else {
    _subscribers.insert({actor, std::make_shared<CarlaEgoVehicleControlSubscriber>(actor, base_topic_name, std::move(frame_id))});
  }
}

void ROS2::UnregisterVehicle(void *actor) {
  _subscribers.erase(actor);
  _actor_callbacks.erase(actor);
  UnregisterSensor(actor);
}

void ROS2::AddActorParentRosName(void *actor, void *parent) {
  auto it = _actor_parents.find(actor);
  if (it != _actor_parents.end()) {
    it->second.push_back(parent);
  } else {
    _actor_parents.insert({actor, {parent}});
  }
}

void ROS2::AddBasicSubscriberCallback(
    [[maybe_unused]] void *actor,
    [[maybe_unused]] std::string ros_name,
    [[maybe_unused]] ActorMessageCallback callback) {
#if defined(WITH_ROS2_DEMO)
  _actor_message_callbacks.insert_or_assign(actor, std::move(callback));
  _basic_subscriber.reset();
  _basic_subscriber = std::make_shared<BasicSubscriber>(actor, ros_name.c_str());
  _basic_subscriber->Init();
#endif
}

void ROS2::RemoveBasicSubscriberCallback([[maybe_unused]] void *actor) {
#if defined(WITH_ROS2_DEMO)
  _basic_subscriber.reset();
  _actor_message_callbacks.erase(actor);
#endif
}

std::string ROS2::LookupRosName(void *actor) const {
  auto it = _registrations.find(actor);
  return it != _registrations.end() ? it->second.ros_name : std::string{};
}

std::string ROS2::LookupFrameId(void *actor) const {
  auto it = _registrations.find(actor);
  return it != _registrations.end() ? it->second.frame_id : std::string{};
}

std::string ROS2::BuildParentChain(void *actor) const {
  auto it = _actor_parents.find(actor);
  if (it == _actor_parents.end()) {
    return std::string{};
  }
  const std::string current_actor_name = LookupRosName(actor);
  std::string parent_name;
  for (auto *parent : it->second) {
    const std::string name = LookupRosName(parent);
    if (name.empty() || name == current_actor_name) {
      continue;
    }
    parent_name = name + '/' + parent_name;
  }
  if (!parent_name.empty() && parent_name.back() == '/') {
    parent_name.pop_back();
  }
  return parent_name;
}

std::string ROS2::BuildBaseTopicName(void *actor) const {
  const std::string ros_name = LookupRosName(actor);
  if (ros_name.empty()) {
    return std::string{};
  }
  const std::string parent_chain = BuildParentChain(actor);
  std::string base_topic_name = "rt/carla/";
  if (!parent_chain.empty()) {
    base_topic_name += parent_chain + "/";
  }
  base_topic_name += ros_name;
  return base_topic_name;
}

void ROS2::ResolveAutoStreamSuffix(
    void *actor,
    const std::string &prefix,
    carla::streaming::detail::stream_id_type id) {
  auto it = _registrations.find(actor);
  if (it == _registrations.end()) {
    return;
  }
  const std::string placeholder = prefix + "__";
  if (it->second.ros_name != placeholder) {
    return;
  }
  std::string resolved = prefix + std::to_string(id);
  it->second.ros_name = resolved;
  if (it->second.frame_id == placeholder) {
    it->second.frame_id = std::move(resolved);
  }
}

template <typename CameraT>
std::shared_ptr<CarlaCameraPublisher> ROS2::GetOrCreateCameraSensor(
    carla::streaming::detail::stream_id_type id,
    void *actor,
    const std::string &default_prefix) {
  auto it_camera = _camera_publishers.find(actor);
  if (it_camera != _camera_publishers.end()) {
    // Enforce the one-actor-one-camera-type invariant on the cache-hit path.
    // RGB/Depth/SS/IS/Normals all alias CarlaRGBCameraPublisher (shared BGRA
    // passthrough), so a hit across those types casts cleanly and is expected.
    // Only an RGB <-> OpticalFlow mismatch fails the cast: that would route
    // optical-flow float bytes through an RGB publisher (or vice versa). Surface
    // it and skip the sample instead of letting the first-created type silently
    // win and corrupt the published image.
    auto typed = std::dynamic_pointer_cast<CameraT>(it_camera->second);
    if (typed == nullptr) {
      log_error(
          "ROS2 camera publisher type mismatch for actor", actor,
          "- the actor was dispatched as two different camera types; ignoring this sample.");
      return nullptr;
    }
    return typed;
  }

  ResolveAutoStreamSuffix(actor, default_prefix, id);
  const std::string base_topic_name = BuildBaseTopicName(actor);
  const std::string frame_id = LookupFrameId(actor);

  auto new_publisher = std::make_shared<CameraT>(base_topic_name, frame_id);
  _camera_publishers.insert({actor, new_publisher});
  return new_publisher;
}

std::shared_ptr<BasePublisher> ROS2::GetOrCreateSensor(
    int type, carla::streaming::detail::stream_id_type id, void *actor) {
  auto it_publishers = _publishers.find(actor);
  if (it_publishers != _publishers.end()) {
    return it_publishers->second;
  }

  // Resolve auto-naming "prefix__" -> "prefix<stream_id>" before computing the
  // topic name. Each enum case names its own prefix so the resolved ros_name
  // stays stable across ticks.
  auto resolve = [this, actor, id](const std::string &prefix) {
    ResolveAutoStreamSuffix(actor, prefix, id);
  };

  std::shared_ptr<BasePublisher> publisher;
  switch (type) {
    case ESensors::CollisionSensor: {
      resolve("collision");
      publisher = std::make_shared<CarlaCollisionPublisher>(
          BuildBaseTopicName(actor), LookupFrameId(actor));
      break;
    }
    case ESensors::DVSCamera: {
      resolve("dvs");
      publisher = std::make_shared<CarlaDVSCameraPublisher>(
          BuildBaseTopicName(actor), LookupFrameId(actor));
      break;
    }
    case ESensors::GnssSensor: {
      resolve("gnss");
      publisher = std::make_shared<CarlaGNSSPublisher>(
          BuildBaseTopicName(actor), LookupFrameId(actor));
      break;
    }
    case ESensors::InertialMeasurementUnit: {
      resolve("imu");
      publisher = std::make_shared<CarlaIMUPublisher>(
          BuildBaseTopicName(actor), LookupFrameId(actor));
      break;
    }
    case ESensors::Radar: {
      resolve("radar");
      publisher = std::make_shared<CarlaRadarPublisher>(
          BuildBaseTopicName(actor), LookupFrameId(actor));
      break;
    }
    case ESensors::RayCastSemanticLidar: {
      resolve("ray_cast_semantic");
      publisher = std::make_shared<CarlaSemanticLidarPublisher>(
          BuildBaseTopicName(actor), LookupFrameId(actor));
      break;
    }
    case ESensors::RayCastLidar: {
      // Both ray-cast and HSS lidars dispatch here; resolve either placeholder.
      resolve("ray_cast");
      resolve("hss_lidar");
      publisher = std::make_shared<CarlaLidarPublisher>(
          BuildBaseTopicName(actor), LookupFrameId(actor));
      break;
    }
    case ESensors::LaneInvasionSensor:
    case ESensors::ObstacleDetectionSensor:
    case ESensors::RssSensor:
    case ESensors::WorldObserver:
    case ESensors::CameraGBufferUint8:
    case ESensors::CameraGBufferFloat:
      // Sensors without a publisher on ue5-dev today; the dispatch in
      // ProcessDataFrom* logs and exits cleanly.
      return nullptr;
    default:
      log_error("ROS2::GetOrCreateSensor: unknown sensor type", type);
      return nullptr;
  }

  if (publisher) {
    _publishers.insert({actor, publisher});
  }
  return publisher;
}

std::shared_ptr<CarlaTransformPublisher> ROS2::GetOrCreateTransformPublisher(void *actor) {
  auto it = _transforms.find(actor);
  if (it != _transforms.end()) {
    return it->second;
  }
  auto registration_it = _registrations.find(actor);
  if (registration_it == _registrations.end() || !registration_it->second.publish_tf) {
    return nullptr;
  }
  auto transform = std::make_shared<CarlaTransformPublisher>();
  _transforms.insert({actor, transform});
  return transform;
}

namespace {

// Builds the parent_frame_id for TF: top-level actors broadcast against
// "map"; child actors broadcast against their direct parent's frame_id.
std::string ParentFrameOrMap(const std::string &parent_chain) {
  return parent_chain.empty() ? std::string{"map"} : parent_chain;
}

}  // namespace

void ROS2::ProcessDataFromCamera(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    int W, int H, float Fov,
    const carla::SharedBufferView buffer,
    void *actor) {
  // Image dimensions + FOV are now read straight from ImageSerializer's
  // per-frame header inside the camera publisher's WriteCameraInfo call;
  // the W/H/Fov arguments survive for ABI compatibility with the
  // Unreal-side dispatcher.
  (void)W;
  (void)H;
  (void)Fov;

  std::shared_ptr<CarlaCameraPublisher> publisher;
  switch (sensor_type) {
    case ESensors::SceneCaptureCamera:
      publisher = GetOrCreateCameraSensor<CarlaRGBCameraPublisher>(stream_id, actor, "rgb");
      break;
    case ESensors::DepthCamera:
      publisher = GetOrCreateCameraSensor<CarlaDepthCameraPublisher>(stream_id, actor, "depth");
      break;
    case ESensors::NormalsCamera:
      publisher = GetOrCreateCameraSensor<CarlaNormalsCameraPublisher>(stream_id, actor, "normals");
      break;
    case ESensors::SemanticSegmentationCamera:
      publisher = GetOrCreateCameraSensor<CarlaSSCameraPublisher>(stream_id, actor, "semantic_segmentation");
      break;
    case ESensors::InstanceSegmentationCamera:
      publisher = GetOrCreateCameraSensor<CarlaISCameraPublisher>(stream_id, actor, "instance_segmentation");
      break;
    case ESensors::OpticalFlowCamera:
      publisher = GetOrCreateCameraSensor<CarlaOpticalFlowCameraPublisher>(
          stream_id, actor, "optical_flow");
      break;
    case ESensors::CollisionSensor:
    case ESensors::RssSensor:
    case ESensors::WorldObserver:
    case ESensors::CameraGBufferUint8:
    case ESensors::CameraGBufferFloat:
    default:
      log_info(
          "Sensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id,
          "buffer.", buffer->size());
      return;
  }

  if (publisher) {
    const auto *header_ptr = buffer->data();
    if (!header_ptr) {
      return;
    }
    if (sensor_type == ESensors::OpticalFlowCamera) {
      const auto *header = reinterpret_cast<
          const carla::sensor::s11n::OpticalFlowImageSerializer::ImageHeader *>(header_ptr);
      publisher->WriteCameraInfo(
          _seconds, _nanoseconds, 0, 0, header->height, header->width, header->fov_angle, true);
      publisher->WriteImage(
          _seconds, _nanoseconds, header->height, header->width,
          buffer->data() + carla::sensor::s11n::OpticalFlowImageSerializer::header_offset);
    } else {
      const auto *header = reinterpret_cast<
          const carla::sensor::s11n::ImageSerializer::ImageHeader *>(header_ptr);
      publisher->WriteCameraInfo(
          _seconds, _nanoseconds, 0, 0, header->height, header->width, header->fov_angle, true);
      publisher->WriteImage(
          _seconds, _nanoseconds, header->height, header->width,
          buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset);
    }
    publisher->Publish();
  }

  if (auto transform_publisher = GetOrCreateTransformPublisher(actor)) {
    transform_publisher->Write(
        _seconds, _nanoseconds,
        ParentFrameOrMap(BuildParentChain(actor)),
        LookupFrameId(actor),
        sensor_transform.location.x, sensor_transform.location.y, sensor_transform.location.z,
        sensor_transform.rotation.pitch, sensor_transform.rotation.yaw, sensor_transform.rotation.roll);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromGNSS(
    uint64_t /*sensor_type*/,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::geom::GeoLocation &data,
    void *actor) {
  if (auto base = GetOrCreateSensor(ESensors::GnssSensor, stream_id, actor)) {
    auto publisher = std::dynamic_pointer_cast<CarlaGNSSPublisher>(base);
    publisher->Write(_seconds, _nanoseconds, data.latitude, data.longitude, data.altitude);
    publisher->Publish();
  }
  if (auto transform_publisher = GetOrCreateTransformPublisher(actor)) {
    transform_publisher->Write(
        _seconds, _nanoseconds,
        ParentFrameOrMap(BuildParentChain(actor)),
        LookupFrameId(actor),
        sensor_transform.location.x, sensor_transform.location.y, sensor_transform.location.z,
        sensor_transform.rotation.pitch, sensor_transform.rotation.yaw, sensor_transform.rotation.roll);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromIMU(
    uint64_t /*sensor_type*/,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    carla::geom::Vector3D accelerometer,
    carla::geom::Vector3D gyroscope,
    float compass,
    void *actor) {
  if (auto base = GetOrCreateSensor(ESensors::InertialMeasurementUnit, stream_id, actor)) {
    auto publisher = std::dynamic_pointer_cast<CarlaIMUPublisher>(base);
    publisher->Write(
        _seconds, _nanoseconds,
        accelerometer.x, accelerometer.y, accelerometer.z,
        gyroscope.x, gyroscope.y, gyroscope.z,
        compass);
    publisher->Publish();
  }
  if (auto transform_publisher = GetOrCreateTransformPublisher(actor)) {
    transform_publisher->Write(
        _seconds, _nanoseconds,
        ParentFrameOrMap(BuildParentChain(actor)),
        LookupFrameId(actor),
        sensor_transform.location.x, sensor_transform.location.y, sensor_transform.location.z,
        sensor_transform.rotation.pitch, sensor_transform.rotation.yaw, sensor_transform.rotation.roll);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromDVS(
    uint64_t /*sensor_type*/,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::SharedBufferView buffer,
    int /*W*/, int /*H*/, float /*Fov*/,
    void *actor) {
  if (auto base = GetOrCreateSensor(ESensors::DVSCamera, stream_id, actor)) {
    auto publisher = std::dynamic_pointer_cast<CarlaDVSCameraPublisher>(base);
    const auto *header = reinterpret_cast<
        const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
    if (!header) {
      return;
    }
    constexpr std::size_t header_offset =
        carla::sensor::s11n::ImageSerializer::header_offset;
    constexpr std::size_t event_size = sizeof(carla::sensor::data::DVSEvent);
    const std::size_t event_count = (buffer->size() - header_offset) / event_size;
    const std::uint8_t *event_bytes = buffer->data() + header_offset;

    publisher->WriteCameraInfo(
        _seconds, _nanoseconds, 0, 0, header->height, header->width, header->fov_angle, true);
    publisher->WriteImage(
        _seconds, _nanoseconds, header->height, header->width,
        event_count, event_bytes, event_size);
    publisher->WritePointCloud(
        _seconds, _nanoseconds, 1, static_cast<std::uint32_t>(event_count), event_bytes);
    publisher->Publish();
  }
  if (auto transform_publisher = GetOrCreateTransformPublisher(actor)) {
    transform_publisher->Write(
        _seconds, _nanoseconds,
        ParentFrameOrMap(BuildParentChain(actor)),
        LookupFrameId(actor),
        sensor_transform.location.x, sensor_transform.location.y, sensor_transform.location.z,
        sensor_transform.rotation.pitch, sensor_transform.rotation.yaw, sensor_transform.rotation.roll);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromLidar(
    uint64_t /*sensor_type*/,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    carla::sensor::data::LidarData &data,
    void *actor) {
  if (auto base = GetOrCreateSensor(ESensors::RayCastLidar, stream_id, actor)) {
    auto publisher = std::dynamic_pointer_cast<CarlaLidarPublisher>(base);
    // The lidar returns a flat list of floats rather than structured detection
    // points. Each detection is 4 floats: x, y, z, intensity. Divide the total
    // float count by 4 to recover the number of detections.
    const auto width = static_cast<std::uint32_t>(data._points.size() / 4u);
    publisher->WritePointCloud(
        _seconds, _nanoseconds, 1u, width,
        reinterpret_cast<const std::uint8_t *>(data._points.data()));
    publisher->Publish();
  }
  if (auto transform_publisher = GetOrCreateTransformPublisher(actor)) {
    transform_publisher->Write(
        _seconds, _nanoseconds,
        ParentFrameOrMap(BuildParentChain(actor)),
        LookupFrameId(actor),
        sensor_transform.location.x, sensor_transform.location.y, sensor_transform.location.z,
        sensor_transform.rotation.pitch, sensor_transform.rotation.yaw, sensor_transform.rotation.roll);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromSemanticLidar(
    uint64_t /*sensor_type*/,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    carla::sensor::data::SemanticLidarData &data,
    void *actor) {
  if (auto base = GetOrCreateSensor(ESensors::RayCastSemanticLidar, stream_id, actor)) {
    auto publisher = std::dynamic_pointer_cast<CarlaSemanticLidarPublisher>(base);
    const auto width = static_cast<std::uint32_t>(data._ser_points.size());
    publisher->WritePointCloud(
        _seconds, _nanoseconds, 1u, width,
        reinterpret_cast<const std::uint8_t *>(data._ser_points.data()));
    publisher->Publish();
  }
  if (auto transform_publisher = GetOrCreateTransformPublisher(actor)) {
    transform_publisher->Write(
        _seconds, _nanoseconds,
        ParentFrameOrMap(BuildParentChain(actor)),
        LookupFrameId(actor),
        sensor_transform.location.x, sensor_transform.location.y, sensor_transform.location.z,
        sensor_transform.rotation.pitch, sensor_transform.rotation.yaw, sensor_transform.rotation.roll);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromRadar(
    uint64_t /*sensor_type*/,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::sensor::data::RadarData &data,
    void *actor) {
  if (auto base = GetOrCreateSensor(ESensors::Radar, stream_id, actor)) {
    auto publisher = std::dynamic_pointer_cast<CarlaRadarPublisher>(base);
    const auto width = static_cast<std::uint32_t>(data.GetDetectionCount());
    publisher->WritePointCloud(
        _seconds, _nanoseconds, 1u, width,
        reinterpret_cast<const std::uint8_t *>(data._detections.data()));
    publisher->Publish();
  }
  if (auto transform_publisher = GetOrCreateTransformPublisher(actor)) {
    transform_publisher->Write(
        _seconds, _nanoseconds,
        ParentFrameOrMap(BuildParentChain(actor)),
        LookupFrameId(actor),
        sensor_transform.location.x, sensor_transform.location.y, sensor_transform.location.z,
        sensor_transform.rotation.pitch, sensor_transform.rotation.yaw, sensor_transform.rotation.roll);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromObstacleDetection(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform /*sensor_transform*/,
    AActor * /*first_actor*/,
    AActor * /*second_actor*/,
    float distance,
    void * /*actor*/) {
  log_info(
      "Sensor ObstacleDetector to ROS data: frame.", _frame, "sensor.", sensor_type,
      "stream.", stream_id, "distance.", distance);
}

void ROS2::ProcessDataFromCollisionSensor(
    uint64_t /*sensor_type*/,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    uint32_t other_actor,
    carla::geom::Vector3D impulse,
    void *actor) {
  if (auto base = GetOrCreateSensor(ESensors::CollisionSensor, stream_id, actor)) {
    auto publisher = std::dynamic_pointer_cast<CarlaCollisionPublisher>(base);
    publisher->Write(_seconds, _nanoseconds, other_actor, impulse.x, impulse.y, impulse.z);
    publisher->Publish();
  }
  if (auto transform_publisher = GetOrCreateTransformPublisher(actor)) {
    transform_publisher->Write(
        _seconds, _nanoseconds,
        ParentFrameOrMap(BuildParentChain(actor)),
        LookupFrameId(actor),
        sensor_transform.location.x, sensor_transform.location.y, sensor_transform.location.z,
        sensor_transform.rotation.pitch, sensor_transform.rotation.yaw, sensor_transform.rotation.roll);
    transform_publisher->Publish();
  }
}

void ROS2::Shutdown() {
  for (auto &element : _publishers) {
    element.second.reset();
  }
  for (auto &element : _transforms) {
    element.second.reset();
  }
  for (auto &element : _camera_publishers) {
    element.second.reset();
  }
  _publishers.clear();
  _transforms.clear();
  _camera_publishers.clear();
  _subscribers.clear();
  _actor_callbacks.clear();
  _registrations.clear();
  _actor_parents.clear();
  _clock_publisher.reset();
  _enabled = false;
#if defined(WITH_ROS2_DEMO)
  _basic_publisher.reset();
  _basic_subscriber.reset();
#endif
}

}  // namespace ros2
}  // namespace carla
