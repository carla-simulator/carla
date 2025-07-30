// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/BufferView.h"
#include "carla/geom/Transform.h"
#include "carla/ros2/ROS2CallbackData.h"
#include "carla/streaming/detail/Types.h"

#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <vector>

// forward declarations
class AActor;
namespace carla {
  namespace geom {
    class GeoLocation;
    class Vector3D;
  }
  namespace sensor {
    namespace data {
      struct DVSEvent;
      class LidarData;
      class SemanticLidarData;
      class RadarData;
    }
  }
}
namespace carla {
namespace ros2 {

  class BasePublisher;
  class BaseSubscriber;

  class CarlaTransformPublisher;
  class CarlaClockPublisher;

class ROS2
{
  public:

    // deleting copy constructor for singleton
    ROS2(const ROS2& obj) = delete;
    static std::shared_ptr<ROS2> GetInstance() {
      if (!_instance)
        _instance = std::shared_ptr<ROS2>(new ROS2);
      return _instance;
    }

    // General
    void Enable(bool enable);
    void Shutdown();

    bool IsEnabled() { return _enabled; }

    void SetFrame(uint64_t frame);
    void SetTimestamp(double timestamp);

    std::string GetActorRosName(void *actor);
    std::string GetActorBaseTopicName(void *actor);

    std::string GetFrameId(void *actor);
    std::string GetParentFrameId(void *actor);

    // Registration
    void RegisterActor(void *actor, std::string ros_name, std::string frame_id, bool publish_tf=true);
    void UnregisterActor(void *actor);

    void RegisterActorParent(void *actor, void *parent);

    void RegisterSensor(void *actor, std::string ros_name, std::string frame_id, bool publish_tf);
    void UnregisterSensor(void *actor);

    void RegisterVehicle(void *actor, std::string ros_name, std::string frame_id, ActorCallback callback);
    void UnregisterVehicle(void *actor);

    // Receiving data to publish
    void ProcessDataFromCamera(
      uint64_t sensor_type,
      const carla::geom::Transform sensor_transform,
      const carla::SharedBufferView buffer,
      void *actor = nullptr);
    void ProcessDataFromGNSS(
      uint64_t sensor_type,
      const carla::geom::Transform sensor_transform,
      const carla::geom::GeoLocation &data,
      void *actor = nullptr);
    void ProcessDataFromIMU(
      uint64_t sensor_type,
      const carla::geom::Transform sensor_transform,
      carla::geom::Vector3D accelerometer,
      carla::geom::Vector3D gyroscope,
      float compass,
      void *actor = nullptr);
    void ProcessDataFromDVS(
      uint64_t sensor_type,
      const carla::geom::Transform sensor_transform,
      const carla::SharedBufferView buffer,
      void *actor = nullptr);
    void ProcessDataFromLidar(
      uint64_t sensor_type,
      const carla::geom::Transform sensor_transform,
      carla::sensor::data::LidarData &data,
      void *actor = nullptr);
    void ProcessDataFromSemanticLidar(
      uint64_t sensor_type,
      const carla::geom::Transform sensor_transform,
      carla::sensor::data::SemanticLidarData &data,
      void *actor = nullptr);
    void ProcessDataFromRadar(
      uint64_t sensor_type,
      const carla::geom::Transform sensor_transform,
      const carla::sensor::data::RadarData &data,
      void *actor = nullptr);
    void ProcessDataFromObstacleDetection(
      uint64_t sensor_type,
      const carla::geom::Transform sensor_transform,
      AActor *first_actor,
      AActor *second_actor,
      float distance,
      void *actor = nullptr);
    void ProcessDataFromCollisionSensor(
      uint64_t sensor_type,
      const carla::geom::Transform sensor_transform,
      uint32_t other_actor,
      carla::geom::Vector3D impulse,
      void* actor);

  private:
    std::shared_ptr<CarlaTransformPublisher> GetOrCreateTransformPublisher(void *actor);
    std::shared_ptr<BasePublisher> GetOrCreateSensor(int type, void* actor);

  // sigleton
  ROS2() {};

  static std::shared_ptr<ROS2> _instance;

  bool _enabled { false };
  uint64_t _frame { 0 };
  int32_t _seconds { 0 };
  uint32_t _nanoseconds { 0 };

  std::shared_ptr<CarlaClockPublisher> _clock_publisher;

  // actor->parent relationship
  std::unordered_map<void *, void *> _actor_parent_map;

  std::unordered_map<void *, std::string> _registered_actors;
  std::unordered_map<void *, std::string> _frame_ids;

  std::unordered_map<void *, std::shared_ptr<BasePublisher>> _publishers;
  std::unordered_multimap<void *, std::shared_ptr<BaseSubscriber>> _subscribers;
  std::unordered_map<void *, ActorCallback> _actor_callbacks;

  std::unordered_map<void *, bool> _tfs;
  std::unordered_map<void *, std::shared_ptr<CarlaTransformPublisher>> _tf_publishers;
};

} // namespace ros2
} // namespace carla
