// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/ROS2.h"

#include "carla/Logging.h"
#include "carla/geom/GeoLocation.h"
#include "carla/geom/Vector3D.h"
#include "carla/sensor/data/DVSEvent.h"
#include "carla/sensor/data/LidarData.h"
#include "carla/sensor/data/SemanticLidarData.h"
#include "carla/sensor/data/RadarData.h"
#include "carla/sensor/data/Image.h"
#include "carla/sensor/s11n/ImageSerializer.h"
#include "carla/sensor/s11n/SensorHeaderSerializer.h"

#include "publishers/CarlaCameraPublisher.h"
#include "publishers/CarlaClockPublisher.h"
#include "publishers/CarlaCollisionPublisher.h"
#include "publishers/CarlaDepthCameraPublisher.h"
#include "publishers/CarlaDVSPublisher.h"
#include "publishers/CarlaGNSSPublisher.h"
#include "publishers/CarlaIMUPublisher.h"
#include "publishers/CarlaISCameraPublisher.h"
#include "publishers/CarlaLidarPublisher.h"
#include "publishers/CarlaNormalsCameraPublisher.h"
#include "publishers/CarlaOpticalFlowCameraPublisher.h"
#include "publishers/CarlaRadarPublisher.h"
#include "publishers/CarlaRGBCameraPublisher.h"
#include "publishers/CarlaSemanticLidarPublisher.h"
#include "publishers/CarlaSSCameraPublisher.h"
#include "publishers/CarlaTransformPublisher.h"

#include "subscribers/AckermannControlSubscriber.h"
#include "subscribers/CarlaEgoVehicleControlSubscriber.h"

#include <vector>

namespace carla {
namespace ros2 {

// static fields
std::shared_ptr<ROS2> ROS2::_instance;

// list of sensors (should be equal to the list of SensorsRegistry
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
  CameraGBufferUint8,
  CameraGBufferFloat
};

void ROS2::Enable(bool enable) {
  _clock_publisher = std::make_shared<CarlaClockPublisher>();

  _enabled = enable;
}

void ROS2::SetFrame(uint64_t frame) {
  _frame = frame;

  for (auto& element : _subscribers) {
    auto actor = element.first;
    auto subscriber = element.second;
    auto callback = _actor_callbacks.find(actor)->second;

    subscriber->ProcessMessages(callback);
  }
}

void ROS2::SetTimestamp(double timestamp) {
  double integral;
  const double fractional = modf(timestamp, &integral);
  const double multiplier = 1000000000.0;
  _seconds = static_cast<int32_t>(integral);
  _nanoseconds = static_cast<uint32_t>(fractional * multiplier);
 
  _clock_publisher->Write(_seconds, _nanoseconds);
  _clock_publisher->Publish();
}

void ROS2::RegisterActor(void *actor, std::string ros_name, std::string frame_id, bool publish_tf) {
  _registered_actors.insert({actor, ros_name});
  _frame_ids.insert({actor, frame_id});
  _tfs.insert({actor, publish_tf});
}

void ROS2::UnregisterActor(void *actor) {
  _registered_actors.erase(actor);
  _frame_ids.erase(actor);
  _actor_parent_map.erase(actor);
  _tfs.erase(actor);
}

void ROS2::RegisterActorParent(void *actor, void *parent) {
  _actor_parent_map.insert({actor, parent});
}

void ROS2::RegisterSensor(void *actor, std::string ros_name, std::string frame_id, bool publish_tf) {
  RegisterActor(actor, ros_name, frame_id, publish_tf);
}

void ROS2::UnregisterSensor(void *actor) {
  UnregisterActor(actor);
  _publishers.erase(actor);
}

void ROS2::RegisterVehicle(void *actor, std::string ros_name, std::string frame_id, ActorCallback callback) {
  RegisterActor(actor, ros_name, frame_id);

  // Register actor callback
  _actor_callbacks.insert({actor, std::move(callback)});

  // Register subscribers
  auto base_topic_name = GetActorBaseTopicName(actor);

  auto _vehicle_control_subscriber = std::make_shared<CarlaEgoVehicleControlSubscriber>(actor, base_topic_name, frame_id);
  _subscribers.insert({actor, _vehicle_control_subscriber});

  auto _ackermann_control_subscriber = std::make_shared<AckermannControlSubscriber>(actor, base_topic_name, frame_id);
  _subscribers.insert({actor, _ackermann_control_subscriber});

}

void ROS2::UnregisterVehicle(void *actor) {
  UnregisterActor(actor);
  _actor_callbacks.erase(actor);
  _subscribers.erase(actor);
}

std::string ROS2::GetActorRosName(void *actor) {
  auto it = _registered_actors.find(actor);
  return it != _registered_actors.end() ? it->second : "";
}

std::string ROS2::GetActorBaseTopicName(void *actor) {
  auto it = _actor_parent_map.find(actor);
  if (it != _actor_parent_map.end()) {
    return GetActorBaseTopicName(it->second) + "/" + GetActorRosName(actor);
  } else {
    return "rt/carla/" + GetActorRosName(actor);
  }
}

std::string ROS2::GetFrameId(void *actor) {
  auto it = _frame_ids.find(actor);
  return it != _frame_ids.end() ? it->second : "";
}

std::string ROS2::GetParentFrameId(void *actor) {
  auto it = _actor_parent_map.find(actor);
  if (it != _actor_parent_map.end()) {
    return GetFrameId(it->second);
  } else {
    return "map";
  }
}

std::shared_ptr<CarlaTransformPublisher> ROS2::GetOrCreateTransformPublisher(void *actor) {

  auto it = _tfs.find(actor);
  if (it == _tfs.end() || it->second == false) {
    return nullptr;
  }

  // Check if the transform publisher is already created
  auto itp = _tf_publishers.find(actor);
  if (itp != _tf_publishers.end()) {
    return itp->second;
  }

  auto tf_publisher = std::make_shared<CarlaTransformPublisher>();
  _tf_publishers.insert({actor, tf_publisher});
  return tf_publisher;
}

std::shared_ptr<BasePublisher> ROS2::GetOrCreateSensor(int type, void* actor) {

  // Check if the sensor publisher is already created
  auto it = _publishers.find(actor);
  if (it != _publishers.end()) {
    return it->second;
  }

  auto create_and_register = [&](auto publisher) {
    _publishers.insert({actor, publisher});
    return publisher;
  };

  std::string topic_name = GetActorBaseTopicName(actor);
  std::string frame_id = GetFrameId(actor);

  switch(type) {
    case ESensors::CollisionSensor:
      return create_and_register(std::make_shared<CarlaCollisionPublisher>(topic_name, frame_id));
    case ESensors::DepthCamera:
      return create_and_register(std::make_shared<CarlaDepthCameraPublisher>(topic_name, frame_id));
    case ESensors::NormalsCamera:
      return create_and_register(std::make_shared<CarlaNormalsCameraPublisher>(topic_name, frame_id));
    case ESensors::DVSCamera:
      return create_and_register(std::make_shared<CarlaDVSPublisher>(topic_name, frame_id));
    case ESensors::GnssSensor:
      return create_and_register(std::make_shared<CarlaGNSSPublisher>(topic_name, frame_id));
    case ESensors::InertialMeasurementUnit:
      return create_and_register(std::make_shared<CarlaIMUPublisher>(topic_name, frame_id));
    case ESensors::OpticalFlowCamera:
      return create_and_register(std::make_shared<CarlaOpticalFlowCameraPublisher>(topic_name, frame_id));
    case ESensors::Radar:
      return create_and_register(std::make_shared<CarlaRadarPublisher>(topic_name, frame_id));
    case ESensors::RayCastSemanticLidar:
      return create_and_register(std::make_shared<CarlaSemanticLidarPublisher>(topic_name, frame_id));
    case ESensors::RayCastLidar:
      return create_and_register(std::make_shared<CarlaLidarPublisher>(topic_name, frame_id));
    case ESensors::SceneCaptureCamera:
      return create_and_register(std::make_shared<CarlaRGBCameraPublisher>(topic_name, frame_id));
    case ESensors::SemanticSegmentationCamera:
      return create_and_register(std::make_shared<CarlaSSCameraPublisher>(topic_name, frame_id));
    case ESensors::InstanceSegmentationCamera:
       return create_and_register(std::make_shared<CarlaISCameraPublisher>(topic_name, frame_id));
    case ESensors::LaneInvasionSensor:
    case ESensors::ObstacleDetectionSensor:
    case ESensors::RssSensor:
    case ESensors::WorldObserver:
    case ESensors::CameraGBufferUint8:
    case ESensors::CameraGBufferFloat:
      return nullptr;
  }
}

void ROS2::ProcessDataFromCamera(
    uint64_t sensor_type,
    const carla::geom::Transform sensor_transform,
    const carla::SharedBufferView buffer,
    void *actor) {

  auto base_publisher = GetOrCreateSensor(sensor_type, actor);
  auto sensor_publisher = std::dynamic_pointer_cast<CarlaCameraPublisher>(base_publisher);
  auto transform_publisher = GetOrCreateTransformPublisher(actor);

  const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
    reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
  if (!header)
    return;

  sensor_publisher->WriteCameraInfo(_seconds, _nanoseconds, 0, 0, header->height, header->width, header->fov_angle, true);
  sensor_publisher->WriteImage(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
  sensor_publisher->Publish();

  if (transform_publisher) {
    transform_publisher->Write(_seconds, _nanoseconds, GetParentFrameId(actor), GetFrameId(actor), sensor_transform);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromGNSS(
    uint64_t sensor_type,
    const carla::geom::Transform sensor_transform,
    const carla::geom::GeoLocation &data,
    void *actor) {
  
  auto base_publisher = GetOrCreateSensor(ESensors::GnssSensor, actor);
  auto sensor_publisher = std::dynamic_pointer_cast<CarlaGNSSPublisher>(base_publisher);
  auto transform_publisher = GetOrCreateTransformPublisher(actor);

  sensor_publisher->Write(_seconds, _nanoseconds, data);
  sensor_publisher->Publish();

  if (transform_publisher) {
    transform_publisher->Write(_seconds, _nanoseconds, GetParentFrameId(actor), GetFrameId(actor), sensor_transform);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromIMU(
    uint64_t sensor_type,
    const carla::geom::Transform sensor_transform,
    carla::geom::Vector3D accelerometer,
    carla::geom::Vector3D gyroscope,
    float compass,
    void *actor) {

  auto base_publisher = GetOrCreateSensor(ESensors::InertialMeasurementUnit, actor);
  auto sensor_publisher = std::dynamic_pointer_cast<CarlaIMUPublisher>(base_publisher);
  auto transform_publisher = GetOrCreateTransformPublisher(actor);

  sensor_publisher->Write(_seconds, _nanoseconds, accelerometer, gyroscope, compass);
  sensor_publisher->Publish();

  if (transform_publisher) {
    transform_publisher->Write(_seconds, _nanoseconds, GetParentFrameId(actor), GetFrameId(actor), sensor_transform);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromDVS(
    uint64_t sensor_type,
    const carla::geom::Transform sensor_transform,
    const carla::SharedBufferView buffer,
    void *actor) {

  auto base_publisher = GetOrCreateSensor(ESensors::DVSCamera, actor);
  auto sensor_publisher = std::dynamic_pointer_cast<CarlaDVSPublisher>(base_publisher);
  auto transform_publisher = GetOrCreateTransformPublisher(actor);

  const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
    reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
  if (!header)
    return;

  const size_t elements =  (buffer->size() - carla::sensor::s11n::ImageSerializer::header_offset) / sizeof(carla::sensor::data::DVSEvent);
  const size_t im_width = header->width;
  const size_t im_height = header->height;

  sensor_publisher->WriteCameraInfo(_seconds, _nanoseconds, 0, 0, im_height, im_width, header->fov_angle, true);
  sensor_publisher->WriteImage(_seconds, _nanoseconds, elements, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
  sensor_publisher->WritePointCloud(_seconds, _nanoseconds, 1, elements, (uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
  sensor_publisher->Publish();

  if (transform_publisher) {
    transform_publisher->Write(_seconds, _nanoseconds, GetParentFrameId(actor), GetFrameId(actor), sensor_transform);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromLidar(
    uint64_t sensor_type,
    const carla::geom::Transform sensor_transform,
    carla::sensor::data::LidarData &data,
    void *actor) {

  auto base_publisher = GetOrCreateSensor(ESensors::RayCastLidar, actor);
  auto sensor_publisher = std::dynamic_pointer_cast<CarlaLidarPublisher>(base_publisher);
  auto transform_publisher = GetOrCreateTransformPublisher(actor);

  // The lidar returns a flat list of floats rather than structured detection points.
  // Each lidar detection consists of 4 floats: x, y, z, and intensity.
  // Divide the total number of floats by 4 to get the number of lidar detections.
  size_t width = data._points.size() / 4;
  size_t height = 1;
  sensor_publisher->WritePointCloud(_seconds, _nanoseconds, height, width, (uint8_t*)data._points.data());
  sensor_publisher->Publish();

  if (transform_publisher) {
    transform_publisher->Write(_seconds, _nanoseconds, GetParentFrameId(actor), GetFrameId(actor), sensor_transform);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromSemanticLidar(
    uint64_t sensor_type,
    const carla::geom::Transform sensor_transform,
    carla::sensor::data::SemanticLidarData &data,
    void *actor) {

  auto base_publisher = GetOrCreateSensor(ESensors::RayCastSemanticLidar, actor);
  auto sensor_publisher = std::dynamic_pointer_cast<CarlaSemanticLidarPublisher>(base_publisher);
  auto transform_publisher = GetOrCreateTransformPublisher(actor);

  size_t width = data._ser_points.size();
  size_t height = 1;
  sensor_publisher->WritePointCloud(_seconds, _nanoseconds, height, width, (uint8_t*)data._ser_points.data());
  sensor_publisher->Publish();

  if (transform_publisher) {
    transform_publisher->Write(_seconds, _nanoseconds, GetParentFrameId(actor), GetFrameId(actor), sensor_transform);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromRadar(
    uint64_t sensor_type,
    const carla::geom::Transform sensor_transform,
    const carla::sensor::data::RadarData &data,
    void *actor) {

  auto base_publisher = GetOrCreateSensor(ESensors::Radar, actor);
  auto sensor_publisher = std::dynamic_pointer_cast<CarlaRadarPublisher>(base_publisher);
  auto transform_publisher = GetOrCreateTransformPublisher(actor);

  size_t width = data.GetDetectionCount();
  size_t height = 1;
  sensor_publisher->WritePointCloud(_seconds, _nanoseconds, height, width, (uint8_t*)data._detections.data());
  sensor_publisher->Publish();

  if (transform_publisher) {
    transform_publisher->Write(_seconds, _nanoseconds, GetParentFrameId(actor), GetFrameId(actor), sensor_transform);
    transform_publisher->Publish();
  }
}

void ROS2::ProcessDataFromObstacleDetection(
    uint64_t sensor_type,
    const carla::geom::Transform sensor_transform,
    AActor *first_ctor,
    AActor *second_actor,
    float distance,
    void *actor) {
  log_info("Sensor ObstacleDetector to ROS data: frame.", _frame, "sensor.", sensor_type, "distance.", distance);
}

void ROS2::ProcessDataFromCollisionSensor(
    uint64_t sensor_type,
    const carla::geom::Transform sensor_transform,
    uint32_t other_actor,
    carla::geom::Vector3D impulse,
    void* actor) {

  auto base_publisher = GetOrCreateSensor(ESensors::CollisionSensor, actor);
  auto sensor_publisher = std::dynamic_pointer_cast<CarlaCollisionPublisher>(base_publisher);
  auto transform_publisher = GetOrCreateTransformPublisher(actor);

  sensor_publisher->Write(_seconds, _nanoseconds, other_actor, impulse);
  sensor_publisher->Publish();

  if (transform_publisher) {
    transform_publisher->Write(_seconds, _nanoseconds, GetParentFrameId(actor), GetFrameId(actor), sensor_transform);
    transform_publisher->Publish();
  }
}

void ROS2::Shutdown() {
  _publishers.clear();
  _subscribers.clear();

  _tf_publishers.clear();

  _clock_publisher.reset();

  _enabled = false;
}

} // namespace ros2
} // namespace carla
