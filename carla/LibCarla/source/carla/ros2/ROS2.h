// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
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

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// forward declarations
class AActor;
namespace carla {
  namespace geom {
    class GeoLocation;
    struct Vector3D;
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
class CarlaCameraPublisher;
class CarlaClockPublisher;
class CarlaTransformPublisher;
class BasicSubscriber;
class BasicPublisher;

class ROS2 {
public:
  // deleting copy constructor for singleton
  ROS2(const ROS2 &obj) = delete;
  static std::shared_ptr<ROS2> GetInstance() {
    if (!_instance)
      _instance = std::shared_ptr<ROS2>(new ROS2);
    return _instance;
  }

  // general
  void Enable(bool enable);
  void Shutdown();
  bool IsEnabled() { return _enabled; }
  void SetFrame(uint64_t frame);
  void SetTimestamp(double timestamp);

  // actor registration API: replaces the legacy AddActorRosName /
  // GetActorRosName / GetActorParentRosName surface that PR-2 stubbed and
  // PR-4 retired. The plugin calls RegisterSensor / RegisterVehicle when an
  // actor spawns and Unregister* when it destroys; ROS2 builds the topic
  // names, owns per-sensor publishers + TF, and routes subscriber
  // callbacks. The vehicle gets exactly one control subscriber: the Ackermann
  // subscriber when enable_ackermann_control is true, otherwise the direct
  // VehicleControl one. The two control topics are mutually exclusive so they
  // cannot contend frame to frame.
  void RegisterSensor(
      void *actor, std::string ros_name, std::string frame_id, bool publish_tf);
  void UnregisterSensor(void *actor);
  void RegisterVehicle(
      void *actor, std::string ros_name, std::string frame_id, ActorCallback callback,
      bool enable_ackermann_control = false);
  void UnregisterVehicle(void *actor);

  // Topic-hierarchy seam used by the plugin's attach_actor path: tells ROS2
  // that `actor` should publish under `parent`'s ros_name prefix. Walking
  // the parent chain is the publisher-side concern.
  void AddActorParentRosName(void *actor, void *parent);

  // Demo subscriber callbacks (only compiled when WITH_ROS2_DEMO).
  void RemoveBasicSubscriberCallback(void *actor);
  void AddBasicSubscriberCallback(
      void *actor, std::string ros_name, ActorMessageCallback callback);

  // enabling streams to publish
  void EnableStream(carla::streaming::detail::stream_id_type id) {
    _publish_stream.insert(id);
  }
  bool IsStreamEnabled(carla::streaming::detail::stream_id_type id) {
    return _publish_stream.count(id) > 0;
  }
  void ResetStreams() { _publish_stream.clear(); }

  // receiving data to publish
  void ProcessDataFromCamera(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      int W, int H, float Fov,
      const carla::SharedBufferView buffer,
      void *actor = nullptr);
  void ProcessDataFromGNSS(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      const carla::geom::GeoLocation &data,
      void *actor = nullptr);
  void ProcessDataFromIMU(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      carla::geom::Vector3D accelerometer,
      carla::geom::Vector3D gyroscope,
      float compass,
      void *actor = nullptr);
  void ProcessDataFromDVS(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      const carla::SharedBufferView buffer,
      int W, int H, float Fov,
      void *actor = nullptr);
  void ProcessDataFromLidar(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      carla::sensor::data::LidarData &data,
      void *actor = nullptr);
  void ProcessDataFromSemanticLidar(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      carla::sensor::data::SemanticLidarData &data,
      void *actor = nullptr);
  void ProcessDataFromRadar(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      const carla::sensor::data::RadarData &data,
      void *actor = nullptr);
  void ProcessDataFromObstacleDetection(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      AActor *first_actor,
      AActor *second_actor,
      float distance,
      void *actor = nullptr);
  void ProcessDataFromCollisionSensor(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      uint32_t other_actor,
      carla::geom::Vector3D impulse,
      void *actor);

private:
  struct ActorRegistration {
    std::string ros_name;
    std::string frame_id;
    bool publish_tf{true};
  };

  // Resolves an actor's `rt/carla/[parent/]ros_name` base topic by walking the
  // parent chain. Returns empty if the actor is not registered.
  std::string BuildBaseTopicName(void *actor) const;
  std::string LookupRosName(void *actor) const;
  std::string LookupFrameId(void *actor) const;
  std::string BuildParentChain(void *actor) const;

  // Lazy-creates the per-sensor publisher matching `type` (an ESensors enum
  // declared in ROS2.cpp). Returns the BasePublisher pointer; the caller
  // dynamic_pointer_casts to the concrete subtype for typed Write() calls.
  std::shared_ptr<BasePublisher> GetOrCreateSensor(
      int type, carla::streaming::detail::stream_id_type id, void *actor);

  // Lazy-creates the per-sensor transform publisher, gated on the actor's
  // publish_tf flag (set at RegisterSensor time). Returns nullptr if the
  // sensor opted out.
  std::shared_ptr<CarlaTransformPublisher> GetOrCreateTransformPublisher(void *actor);

  // Camera-side counterpart of GetOrCreateSensor for publishers that inherit
  // CarlaCameraPublisher (the RGB / Depth / SS / IS / Normals / OpticalFlow
  // unified base). DVS uses its own composite via GetOrCreateSensor.
  template <typename CameraT>
  std::shared_ptr<CarlaCameraPublisher> GetOrCreateCameraSensor(
      carla::streaming::detail::stream_id_type id,
      void *actor,
      const std::string &default_prefix);

  // Resolves a `prefix__` placeholder by appending the stream id, persisting
  // the resolved name in `_registrations` so subsequent lookups see it.
  void ResolveAutoStreamSuffix(
      void *actor, const std::string &prefix, carla::streaming::detail::stream_id_type id);

  // singleton
  ROS2() = default;

  static std::shared_ptr<ROS2> _instance;

  bool _enabled{false};
  uint64_t _frame{0};
  int32_t _seconds{0};
  uint32_t _nanoseconds{0};

  std::unordered_map<void *, ActorRegistration> _registrations;
  std::unordered_map<void *, std::vector<void *>> _actor_parents;
  std::shared_ptr<CarlaClockPublisher> _clock_publisher;
  std::unordered_map<void *, std::shared_ptr<BasePublisher>> _publishers;
  std::unordered_map<void *, std::shared_ptr<CarlaCameraPublisher>> _camera_publishers;
  std::unordered_map<void *, std::shared_ptr<CarlaTransformPublisher>> _transforms;
  std::unordered_set<carla::streaming::detail::stream_id_type> _publish_stream;
  std::unordered_map<void *, ActorCallback> _actor_callbacks;
  std::unordered_multimap<void *, std::shared_ptr<BaseSubscriber>> _subscribers;
#if defined(WITH_ROS2_DEMO)
  std::shared_ptr<BasicSubscriber> _basic_subscriber;
  std::shared_ptr<BasicPublisher> _basic_publisher;
  std::unordered_map<void *, ActorMessageCallback> _actor_message_callbacks;
#endif
};

}  // namespace ros2
}  // namespace carla
