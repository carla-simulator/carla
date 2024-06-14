// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace ros2 {
namespace types {

enum class PublisherSensorType {
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
  CameraGBufferFloat,
  V2XCustom,
  Unknown
};
}
}  // namespace ros2
}  // namespace carla

namespace std {
inline std::string to_string(carla::ros2::types::PublisherSensorType sensor_type) {
  switch (sensor_type) {
    case carla::ros2::types::PublisherSensorType::CollisionSensor:
      return "CollisionSensor";
    case carla::ros2::types::PublisherSensorType::DepthCamera:
      return "DepthCamera";
    case carla::ros2::types::PublisherSensorType::NormalsCamera:
      return "NormalsCamera";
    case carla::ros2::types::PublisherSensorType::DVSCamera:
      return "DVSCamera";
    case carla::ros2::types::PublisherSensorType::GnssSensor:
      return "GnssSensor";
    case carla::ros2::types::PublisherSensorType::InertialMeasurementUnit:
      return "InertialMeasurementUnit";
    case carla::ros2::types::PublisherSensorType::LaneInvasionSensor:
      return "LaneInvasionSensor";
    case carla::ros2::types::PublisherSensorType::ObstacleDetectionSensor:
      return "ObstacleDetectionSensor";
    case carla::ros2::types::PublisherSensorType::OpticalFlowCamera:
      return "OpticalFlowCamera";
    case carla::ros2::types::PublisherSensorType::Radar:
      return "Radar";
    case carla::ros2::types::PublisherSensorType::RayCastSemanticLidar:
      return "RayCastSemanticLidar";
    case carla::ros2::types::PublisherSensorType::RayCastLidar:
      return "RayCastLidar";
    case carla::ros2::types::PublisherSensorType::RssSensor:
      return "RssSensor";
    case carla::ros2::types::PublisherSensorType::SceneCaptureCamera:
      return "SceneCaptureCamera";
    case carla::ros2::types::PublisherSensorType::SemanticSegmentationCamera:
      return "SemanticSegmentationCamera";
    case carla::ros2::types::PublisherSensorType::InstanceSegmentationCamera:
      return "InstanceSegmentationCamera";
    case carla::ros2::types::PublisherSensorType::WorldObserver:
      return "WorldObserver";
    case carla::ros2::types::PublisherSensorType::CameraGBufferUint8:
      return "CameraGBufferUint8";
    case carla::ros2::types::PublisherSensorType::CameraGBufferFloat:
      return "CameraGBufferFloat";
    case carla::ros2::types::PublisherSensorType::V2XCustom:
      return "V2XCustom";
    default:
      return "Unknown";
  }
}
}  // namespace std
