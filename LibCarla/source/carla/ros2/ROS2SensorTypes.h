// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace ros2 {

  /// Sensor type ids as the ROS 2 interface sees them.
  ///
  /// Cameras reach ROS2::ProcessDataFromCamera with the sensor type from their
  /// data header, which is the sensor's position in carla::sensor::SensorRegistry.
  /// Each value here must therefore be that position: one entry per registry
  /// item, in the same order, including sensors that ROS 2 does not publish.
  /// SensorFactory.cpp checks every entry against the registry at compile time,
  /// so a sensor added to the registry without a matching entry here fails the
  /// build instead of silently shifting every camera after it.
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
    SceneCaptureCamera_RayTracedLens,
    SemanticSegmentationCamera,
    InstanceSegmentationCamera,
    WorldObserver,
    SceneCaptureCamera_WideAngleLens,
    DepthCamera_WideAngleLens,
    InstanceSegmentationCamera_WideAngleLens,
    SemanticSegmentationCamera_WideAngleLens,
    CameraGBufferUint8,
    CameraGBufferFloat,
    HSSLidar,
    V2XSensor,
    CustomV2XSensor,
    VehicleStatusSensor,
    AutowareGnssSensor,
    SceneCaptureCamera_RayTracedLensDistance,
    SceneCaptureCamera_RayTracedLensInstance,
    SensorTypeCount
  };

} // namespace ros2
} // namespace carla
