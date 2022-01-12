// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef LIBCARLA_SENSOR_REGISTRY_INCLUDE_H
#define LIBCARLA_SENSOR_REGISTRY_INCLUDE_H

#include "carla/sensor/CompositeSerializer.h"

// =============================================================================
// Follow the 4 steps to register a new sensor.
// =============================================================================

// 1. Include the serializer here.
#include "carla/sensor/s11n/CollisionEventSerializer.h"
#include "carla/sensor/s11n/DVSEventArraySerializer.h"
#include "carla/sensor/s11n/EpisodeStateSerializer.h"
#include "carla/sensor/s11n/GnssSerializer.h"
#include "carla/sensor/s11n/ImageSerializer.h"
#include "carla/sensor/s11n/OpticalFlowImageSerializer.h"
#include "carla/sensor/s11n/IMUSerializer.h"
#include "carla/sensor/s11n/LidarSerializer.h"
#include "carla/sensor/s11n/NoopSerializer.h"
#include "carla/sensor/s11n/ObstacleDetectionEventSerializer.h"
#include "carla/sensor/s11n/RadarSerializer.h"
#include "carla/sensor/s11n/SemanticLidarSerializer.h"

// 2. Add a forward-declaration of the sensor here.
class ACollisionSensor;
class ADepthCamera;
class ADVSCamera;
class AGnssSensor;
class AInertialMeasurementUnit;
class ALaneInvasionSensor;
class AObstacleDetectionSensor;
class AOpticalFlowCamera;
class ARadar;
class ARayCastSemanticLidar;
class ARayCastLidar;
class ASceneCaptureCamera;
class ASemanticSegmentationCamera;
class AInstanceSegmentationCamera;
class ARssSensor;
class FWorldObserver;

namespace carla {
namespace sensor {

  // 3. Register the sensor and its serializer in the SensorRegistry.

  /// Contains a registry of all the sensors available and allows serializing
  /// and deserializing sensor data for the types registered.
  ///
  /// Use s11n::NoopSerializer if the sensor does not send data (sensors that
  /// work only on client-side).
  using SensorRegistry = CompositeSerializer<
    std::pair<ACollisionSensor *, s11n::CollisionEventSerializer>,
    std::pair<ADepthCamera *, s11n::ImageSerializer>,
    std::pair<ADVSCamera *, s11n::DVSEventArraySerializer>,
    std::pair<AGnssSensor *, s11n::GnssSerializer>,
    std::pair<AInertialMeasurementUnit *, s11n::IMUSerializer>,
    std::pair<ALaneInvasionSensor *, s11n::NoopSerializer>,
    std::pair<AObstacleDetectionSensor *, s11n::ObstacleDetectionEventSerializer>,
    std::pair<AOpticalFlowCamera *, s11n::OpticalFlowImageSerializer>,
    std::pair<ARadar *, s11n::RadarSerializer>,
    std::pair<ARayCastSemanticLidar *, s11n::SemanticLidarSerializer>,
    std::pair<ARayCastLidar *, s11n::LidarSerializer>,
    std::pair<ARssSensor *, s11n::NoopSerializer>,
    std::pair<ASceneCaptureCamera *, s11n::ImageSerializer>,
    std::pair<ASemanticSegmentationCamera *, s11n::ImageSerializer>,
    std::pair<AInstanceSegmentationCamera *, s11n::ImageSerializer>,
    std::pair<FWorldObserver *, s11n::EpisodeStateSerializer>
  >;

} // namespace sensor
} // namespace carla

#endif // LIBCARLA_SENSOR_REGISTRY_INCLUDE_H

#ifdef LIBCARLA_SENSOR_REGISTRY_WITH_SENSOR_INCLUDES

// 4. Include the sensor here.
#include "Carla/Sensor/CollisionSensor.h"
#include "Carla/Sensor/DepthCamera.h"
#include "Carla/Sensor/DVSCamera.h"
#include "Carla/Sensor/GnssSensor.h"
#include "Carla/Sensor/InertialMeasurementUnit.h"
#include "Carla/Sensor/LaneInvasionSensor.h"
#include "Carla/Sensor/ObstacleDetectionSensor.h"
#include "Carla/Sensor/OpticalFlowCamera.h"
#include "Carla/Sensor/Radar.h"
#include "Carla/Sensor/RayCastLidar.h"
#include "Carla/Sensor/RayCastSemanticLidar.h"
#include "Carla/Sensor/RssSensor.h"
#include "Carla/Sensor/SceneCaptureCamera.h"
#include "Carla/Sensor/SemanticSegmentationCamera.h"
#include "Carla/Sensor/InstanceSegmentationCamera.h"
#include "Carla/Sensor/WorldObserver.h"

#endif // LIBCARLA_SENSOR_REGISTRY_WITH_SENSOR_INCLUDES
