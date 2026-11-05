// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/BufferView.h"
#include "carla/geom/Transform.h"
#include "carla/ros2/ROS2CallbackData.h"
#include "carla/ros2/middleware/Middleware.h"
#include "carla/ros2/middleware/MiddlewareConfig.h"
#include "carla/streaming/detail/Types.h"

#include <mutex>
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
  namespace rpc {
    class VehicleControl;
    class VehiclePhysicsControl;
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
  class CarlaMapPublisher;
  class CarlaOdometryPublisher;
  class CarlaEgoVehicleStatusPublisher;
  class CarlaEgoVehicleInfoPublisher;
  class CarlaStaticTransformPublisher;

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
    // Returns true when enabling succeeds (middleware compiled in), false otherwise.
    // Callers pass enable=false to shut down; the return value is always true in that case.
    // domain_id selects the ROS 2 domain id for the chosen middleware; kUnsetDomainId
    // (the default) keeps each middleware's native default.
    bool Enable(bool enable, Middleware middleware = Middleware::FastDDS,
        int domain_id = kUnsetDomainId);
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

    // True when RegisterVehicle created the per-vehicle data publishers for
    // this actor and UnregisterVehicle has not destroyed them yet.
    bool IsVehicleRegistered(void *actor) const;

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
    // Publishes the OpenDRIVE description of the current map as a latched
    // topic. Called once per episode; re-publishing refreshes the latched
    // sample after a map change.
    void ProcessDataFromMap(const std::string &open_drive);
    // Publishes odometry, vehicle status and the dynamic odom -> <vehicle>
    // transform for a registered vehicle. Called once per frame.
    void ProcessDataFromVehicle(
      void *actor,
      const carla::geom::Transform vehicle_transform,
      carla::geom::Vector3D velocity,
      carla::geom::Vector3D angular_velocity,
      float delta_seconds,
      const carla::rpc::VehicleControl &control);
    // Publishes the latched static description of a registered vehicle.
    // Called once at registration.
    void ProcessVehicleInfo(
      void *actor,
      uint32_t id,
      const std::string &type_id,
      const std::string &role_name,
      const carla::geom::Transform vehicle_transform,
      const carla::rpc::VehiclePhysicsControl &physics_control);

  private:
    std::shared_ptr<CarlaTransformPublisher> GetOrCreateTransformPublisher(void *actor);
    std::shared_ptr<BasePublisher> GetOrCreateSensor(int type, void* actor);

  // sigleton
  ROS2() {};

  static std::shared_ptr<ROS2> _instance;

  // Protects all map members from concurrent access by the UE4 tick thread
  // (ProcessDataFrom*, SetFrame) and the RPC thread (Register*, Unregister*).
  // recursive_mutex is required because RegisterSensor calls RegisterActor,
  // and UnregisterSensor calls UnregisterActor.
  mutable std::recursive_mutex _mutex;

  bool _enabled { false };
  uint64_t _frame { 0 };
  int32_t _seconds { 0 };
  uint32_t _nanoseconds { 0 };

  std::shared_ptr<CarlaClockPublisher> _clock_publisher;
  std::shared_ptr<CarlaMapPublisher> _map_publisher;

  // actor->parent relationship
  std::unordered_map<void *, void *> _actor_parent_map;

  std::unordered_map<void *, std::string> _registered_actors;
  std::unordered_map<void *, std::string> _frame_ids;

  std::unordered_map<void *, std::shared_ptr<BasePublisher>> _publishers;
  std::unordered_multimap<void *, std::shared_ptr<BaseSubscriber>> _subscribers;
  std::unordered_map<void *, ActorCallback> _actor_callbacks;

  std::unordered_map<void *, bool> _tfs;
  std::unordered_map<void *, std::shared_ptr<CarlaTransformPublisher>> _tf_publishers;

  // Per-vehicle data publishers, created at RegisterVehicle and destroyed at
  // UnregisterVehicle/Shutdown.
  struct VehiclePublishers {
    std::shared_ptr<CarlaOdometryPublisher> odometry;
    std::shared_ptr<CarlaEgoVehicleStatusPublisher> status;
    std::shared_ptr<CarlaEgoVehicleInfoPublisher> info;
  };
  std::unordered_map<void *, VehiclePublishers> _vehicle_publishers;
  std::shared_ptr<CarlaStaticTransformPublisher> _static_tf_publisher;
};

} // namespace ros2
} // namespace carla
