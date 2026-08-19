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
#include "carla/sensor/s11n/VehicleStatusSerializer.h"

#include "carla/ros2/middleware/ActiveMiddleware.h"

#include "publishers/AutowareGNSSPublisher.h"
#include "publishers/AutowareVehicleStatusPublisher.h"
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
#include "subscribers/AutowareControlSubscriber.h"
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
  HSSLidar,
  // Autoware vehicle interface (appended at the END, matching the
  // SensorRegistry append; V2X sensors have no ROS2 dispatch and therefore
  // no enum entry, mirroring the pre-existing convention).
  VehicleStatusSensor,
  AutowareGnssSensor
};

bool ROS2::Enable(bool enable, Middleware middleware, int domain_id) {
  // Select the ROS 2 middleware before any publisher or subscriber is created.
  // SetActiveMiddleware is the DDS-free bridge that resolves availability inside
  // the shared library (the CARLA_ROS2_MIDDLEWARE_* macros are not visible here
  // in carla-server) and keeps vendor headers out of carla-server.
  if (enable && !SetActiveMiddleware(middleware)) {
    log_error("ROS2: requested middleware '", MiddlewareToString(middleware),
              "' is not available. Compiled in: ", GetAvailableMiddleware(), ".");
    _enabled = false;
    return false;
  }
  if (enable) {
    // Configure the domain id before any transport context is created (the
    // shared participants/sessions are created lazily on the first publisher,
    // i.e. the clock publisher below). SetActiveDomainId is the DDS-free bridge
    // so the value lands in the shared library's MiddlewareConfig, which the
    // middlewares read.
    const ResolvedDomainId resolved = SetActiveDomainId(domain_id);
    const char* domain_source =
        (resolved.source == DomainIdSource::CommandLine)  ? "--ros-domain-id"
        : (resolved.source == DomainIdSource::Environment) ? "ROS_DOMAIN_ID"
                                                           : "default";
    log_info("ROS2: using middleware '", MiddlewareToString(middleware),
        "', domain id: ", resolved.id, " (", domain_source, ")");
  }
  _enabled = enable;
  log_info("ROS2 enabled: ", _enabled);
  _clock_publisher = std::make_shared<CarlaClockPublisher>();
#if defined(WITH_ROS2_DEMO)
  _basic_publisher = std::make_shared<BasicPublisher>();
  _basic_publisher->Init();
#endif
  return true;
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
    bool enable_ackermann_control,
    bool enable_autoware_control) {
  _registrations.insert_or_assign(
      actor, ActorRegistration{ros_name, frame_id, true});

  // Idempotency: drop any prior subscribers / callbacks bound to this actor
  // so a re-registration does not accumulate duplicate DataReaders nor leave
  // the previous callback wired.
  _subscribers.erase(actor);
  _autoware_vehicles.erase(actor);
  _actor_callbacks.insert_or_assign(actor, std::move(callback));

  // The legacy CarlaEgoVehicleControlSubscriber::Init built its topic as
  // "rt/carla/" + [parent + "/"] + name + "/vehicle_control_cmd". With the
  // new template constructors the suffix is appended inside each subscriber,
  // so we hand them the base path only. BuildBaseTopicName also honors an
  // exact-topic override registered via AddActorRosTopicName.
  const std::string base_topic_name = BuildBaseTopicName(actor);

  // The control modes are mutually exclusive: a vehicle listens on exactly one
  // command surface, so two topics can never contend frame to frame. The
  // Autoware command set wins over Ackermann (tier4 gave it priority over the
  // ego control subscriber); Ackermann stays opt-in per youtalk's review.
  if (enable_autoware_control) {
    _autoware_vehicles.insert(actor);
    _subscribers.insert({actor, std::make_shared<AutowareControlSubscriber>(actor, std::move(frame_id))});
  } else if (enable_ackermann_control) {
    _subscribers.insert({actor, std::make_shared<AckermannControlSubscriber>(actor, base_topic_name, std::move(frame_id))});
  } else {
    _subscribers.insert({actor, std::make_shared<CarlaEgoVehicleControlSubscriber>(actor, base_topic_name, std::move(frame_id))});
  }
}

void ROS2::UnregisterVehicle(void *actor) {
  _subscribers.erase(actor);
  _actor_callbacks.erase(actor);
  _autoware_vehicles.erase(actor);
  _autoware_status_publishers.erase(actor);
  UnregisterSensor(actor);
}

void ROS2::AddActorRosTopicName(void *actor, std::string ros_topic_name) {
  _actor_ros_topic_names.insert_or_assign(actor, std::move(ros_topic_name));
}

void ROS2::RemoveActorRosTopicName(void *actor) {
  _actor_ros_topic_names.erase(actor);
  // Drop the cached publishers bound to the overridden topic so the next data
  // tick re-creates them under the default naming (tier4 semantics).
  _publishers.erase(actor);
  _camera_publishers.erase(actor);
  _transforms.erase(actor);
}

std::string ROS2::GetActorRosTopicName(void *actor) const {
  auto it = _actor_ros_topic_names.find(actor);
  return it != _actor_ros_topic_names.end() ? it->second : std::string{};
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
  // Exact-topic override (ros_topic_name attribute flow): a non-empty custom
  // name replaces the whole "rt/carla/[parent/]ros_name" convention. Suffixes
  // for multi-topic sensors are appended by the publishers on top of this.
  auto override_it = _actor_ros_topic_names.find(actor);
  if (override_it != _actor_ros_topic_names.end() && !override_it->second.empty()) {
    const std::string &custom = override_it->second;
    std::string base_topic_name = "rt";
    if (custom.front() != '/') {
      base_topic_name += '/';
    }
    base_topic_name += custom;
    return base_topic_name;
  }

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
    case ESensors::AutowareGnssSensor: {
      resolve("autoware_gnss");
      // frame_id "map" (not the sensor's own frame): the published poses are
      // world/map-frame quantities including the MGRS offset. Architecture
      // decision — tier4's AutowarePublisherBase used the sensor ros_name as
      // frame_id, which was accidental; triage confirmed "map" is the intent.
      publisher = std::make_shared<AutowareGNSSPublisher>(
          BuildBaseTopicName(actor), "map");
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
  // Global TF gate (tier4's _publish_tf): checked before the cache so
  // toggling at runtime silences every sensor immediately.
  if (!_publish_tf) {
    return nullptr;
  }
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
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    carla::sensor::data::LidarData &data,
    void *actor) {
  // No sensor description available: plain XYZI layout (channel_count = 0
  // routes the extended path off).
  ProcessDataFromLidar(
      sensor_type, stream_id, sensor_transform,
      0u, 0.0f, 0.0f, data, actor);
}

void ROS2::ProcessDataFromLidar(
    uint64_t /*sensor_type*/,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    uint32_t channel_count,
    float upper_fov_limit,
    float lower_fov_limit,
    carla::sensor::data::LidarData &data,
    void *actor) {
  if (auto base = GetOrCreateSensor(ESensors::RayCastLidar, stream_id, actor)) {
    auto publisher = std::dynamic_pointer_cast<CarlaLidarPublisher>(base);
    // The lidar returns a flat list of floats rather than structured detection
    // points. Each detection is 4 floats: x, y, z, intensity. Divide the total
    // float count by 4 to recover the number of detections.
    const auto width = static_cast<std::uint32_t>(data._points.size() / 4u);
    if (channel_count >= 2u) {
      // tier4 routing: every raycast lidar whose description is known goes
      // through the extended Autoware XYZIRCAEDT layout. Per-channel
      // elevations span [upper_fov, lower_fov] (degrees in, radians out),
      // uniformly spaced, top channel first — same as tier4's ROS2.cpp.
      const float upper_rad = upper_fov_limit * static_cast<float>(M_PI) / 180.0f;
      const float lower_rad = lower_fov_limit * static_cast<float>(M_PI) / 180.0f;
      std::vector<float> vertical_angles;
      vertical_angles.reserve(channel_count);
      const float vertical_step =
          (upper_rad - lower_rad) / static_cast<float>(channel_count - 1u);
      for (uint32_t i = 0u; i < channel_count; ++i) {
        vertical_angles.push_back(upper_rad - static_cast<float>(i) * vertical_step);
      }
      publisher->WriteExtendedPointCloud(
          _seconds, _nanoseconds, 1u, width,
          reinterpret_cast<const std::uint8_t *>(data._points.data()),
          data._header.data() + carla::sensor::data::LidarData::Index::SIZE,
          data._header.size() - carla::sensor::data::LidarData::Index::SIZE,
          vertical_angles);
    } else {
      publisher->WritePointCloud(
          _seconds, _nanoseconds, 1u, width,
          reinterpret_cast<const std::uint8_t *>(data._points.data()));
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

void ROS2::ProcessDataFromStatusSensor(
    uint64_t /*sensor_type*/,
    carla::streaming::detail::stream_id_type /*stream_id*/,
    const carla::geom::Transform /*sensor_transform*/,
    const carla::sensor::s11n::VehicleStatusData &data,
    void *vehicle_actor,
    void * /*actor*/) {
  // Only vehicles registered with enable_autoware_control publish reports:
  // the report topics are fixed absolute names shared by the whole vehicle
  // interface (tier4 restricted this to the ego for the same reason).
  if (_autoware_vehicles.count(vehicle_actor) == 0) {
    log_warning(
        "ROS2::ProcessDataFromStatusSensor: the Vehicle Status Sensor is attached to a "
        "vehicle that is not registered for Autoware control - not publishing the data.");
    return;
  }

  std::shared_ptr<AutowareVehicleStatusPublisher> publisher;
  auto it = _autoware_status_publishers.find(vehicle_actor);
  if (it != _autoware_status_publishers.end()) {
    publisher = it->second;
  } else {
    publisher = std::make_shared<AutowareVehicleStatusPublisher>();
    _autoware_status_publishers.insert({vehicle_actor, publisher});
  }

  constexpr uint8_t reverse_mask     = 0b00000001;
  constexpr uint8_t manual_gear_mask = 0b00000010;

  constexpr uint8_t left_blinker_mask  = 0b00000001;
  constexpr uint8_t right_blinker_mask = 0b00000010;
  constexpr uint8_t hazard_lights_mask = 0b00000100;

  // Decode data (control_flags b0=reverse b1=manual; turn_mask b0=L b1=R b2=hazard).
  const bool is_reverse = (data.control_flags & reverse_mask) != 0;
  const bool is_manual_gear = (data.control_flags & manual_gear_mask) != 0;
  (void)is_reverse;      // reserved: tier4 TODO — gear/reverse interplay
  (void)is_manual_gear;  // reserved: tier4 TODO

  const bool is_left_blinker_on = (data.turn_mask & left_blinker_mask) != 0;
  const bool is_right_blinker_on = (data.turn_mask & right_blinker_mask) != 0;
  const bool is_hazard_lights_on = (data.turn_mask & hazard_lights_mask) != 0;

  // Positive values mean forward and left (when going forward), so the local
  // Y velocity and Z angular velocity flip sign from CARLA's convention.
  publisher->SetVelocity(data.vel_x_mps, -data.vel_y_mps, -data.angVel_z_mps);

  // Steering is reported negated, matching the negation applied when the
  // Autoware steering command is translated to CARLA (tier4 parity).
  publisher->SetSteering(-data.steer);

  // Control mode command is a ROS service in Autoware, so there is no easy
  // way to reflect it yet; AUTONOMOUS is reported unconditionally (tier4).
  publisher->SetControlMode(ControlMode::AUTONOMOUS);

  publisher->SetGear(Gear::NONE);
  switch (data.gear) {
#define CASE(GEAR_VALUE, GEAR_ENUM)   \
  case GEAR_VALUE:                    \
    publisher->SetGear(GEAR_ENUM);    \
    break;                            \
    static_assert(true, "")

    CASE(-2,  Gear::REVERSE_2);
    CASE(-1,  Gear::REVERSE  );
    CASE( 0,  Gear::NEUTRAL  );
    CASE( 1,  Gear::DRIVE    );
    CASE( 2,  Gear::DRIVE_2  );
    CASE( 3,  Gear::DRIVE_3  );
    CASE( 4,  Gear::DRIVE_4  );
    CASE( 5,  Gear::DRIVE_5  );
    CASE( 6,  Gear::DRIVE_6  );
    CASE( 7,  Gear::DRIVE_7  );
    CASE( 8,  Gear::DRIVE_8  );
    CASE( 9,  Gear::DRIVE_9  );
    CASE( 10, Gear::DRIVE_10 );
    CASE( 11, Gear::DRIVE_11 );
    CASE( 12, Gear::DRIVE_12 );
    CASE( 13, Gear::DRIVE_13 );
    CASE( 14, Gear::DRIVE_14 );
    CASE( 15, Gear::DRIVE_15 );
    CASE( 16, Gear::DRIVE_16 );
    CASE( 17, Gear::DRIVE_17 );
    CASE( 18, Gear::DRIVE_18 );

#undef CASE
  }

  // Turn indicators.
  if (!is_left_blinker_on && !is_right_blinker_on) {
    publisher->SetTurnIndicators(TurnIndicatorsStatus::OFF);
  } else if (is_left_blinker_on && !is_right_blinker_on) {
    publisher->SetTurnIndicators(TurnIndicatorsStatus::LEFT);
  } else if (is_right_blinker_on && !is_left_blinker_on) {
    publisher->SetTurnIndicators(TurnIndicatorsStatus::RIGHT);
  } else {
    log_error("ROS2::ProcessDataFromStatusSensor: both blinkers are on; this should not happen.");
  }

  publisher->SetHazardLights(is_hazard_lights_on);

  publisher->Publish(_seconds, _nanoseconds);
}

void ROS2::ProcessDataFromAutowareGNSS(
    uint64_t /*sensor_type*/,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::geom::GeoLocation & /*data*/,
    const carla::geom::Transform &sensor_world_transform,
    const double mgrs_offset_position[3],
    void *actor) {
  if (auto base = GetOrCreateSensor(ESensors::AutowareGnssSensor, stream_id, actor)) {
    if (auto publisher = std::dynamic_pointer_cast<AutowareGNSSPublisher>(base)) {
      publisher->Write(
          _seconds, _nanoseconds,
          sensor_world_transform.location.x,
          sensor_world_transform.location.y,
          sensor_world_transform.location.z,
          sensor_world_transform.rotation.pitch,
          sensor_world_transform.rotation.yaw,
          sensor_world_transform.rotation.roll,
          mgrs_offset_position[0], mgrs_offset_position[1], mgrs_offset_position[2]);
      publisher->Publish();
    } else {
      log_error(
          "ROS2::ProcessDataFromAutowareGNSS: publisher type mismatch for actor", actor,
          "- the actor was dispatched as both regular and Autoware GNSS; ignoring this sample.");
    }
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
  _actor_ros_topic_names.clear();
  _autoware_vehicles.clear();
  _autoware_status_publishers.clear();
  _clock_publisher.reset();
  _enabled = false;
#if defined(WITH_ROS2_DEMO)
  _basic_publisher.reset();
  _basic_subscriber.reset();
#endif
}

}  // namespace ros2
}  // namespace carla
