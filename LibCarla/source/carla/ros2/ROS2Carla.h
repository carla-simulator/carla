// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/ROS2Interfaces.h"
#include "carla/Buffer.h"
#include "carla/BufferView.h"
#include "carla/geom/Transform.h"
#include "carla/ros2/ROS2.h"
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

  class CarlaPublisher;
  class CarlaTransformPublisher;
  class CarlaClockPublisher;
  class CarlaEgoVehicleControlSubscriber;
  class BasicSubscriber;
  class BasicPublisher;

class ROS2Carla : public ROS2
{
  public:

  // deleting copy constructor for singleton
  ROS2Carla(const ROS2Carla& obj) = delete;
  static std::shared_ptr<ROS2Carla> GetInstance() {
    if (!_instance)
    {
      _instance = std::shared_ptr<ROS2Carla>(new ROS2Carla);
      auto ROS2Interfaces = carla::ros2::ROS2Interfaces::GetInstance();
      ROS2Interfaces->RegisterInterface(_instance);
    }
    return _instance;
  }

  virtual void Enable(bool enable) override;
  virtual void Shutdown() override;
  virtual bool IsEnabled() override { return _enabled; }
  virtual void SetFrame(uint64_t frame) override;
  virtual void SetTimestamp(double timestamp) override;

  // ros_name managing
  virtual void RemoveActorRosPublishers(void *actor) override;

  // callbacks
  void AddActorCallback(void* actor, std::string ros_name, ActorCallback callback);
  void RemoveActorCallback(void* actor);
  void RemoveBasicSubscriberCallback(void* actor);
  void AddBasicSubscriberCallback(void* actor, std::string ros_name, ActorMessageCallback callback);

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
      void* actor);

    // enabling streams to publish
  virtual void EnableStream(carla::streaming::detail::stream_id_type id) { _publish_stream.insert(id); }
  virtual bool IsStreamEnabled(carla::streaming::detail::stream_id_type id) { return _publish_stream.count(id) > 0; }
  virtual void ResetStreams() { _publish_stream.clear(); }

  static std::shared_ptr<ROS2Carla> _instance;

  private:
  std::pair<std::shared_ptr<CarlaPublisher>, std::shared_ptr<CarlaTransformPublisher>> GetOrCreateSensor(int type, carla::streaming::detail::stream_id_type id, void* actor);

  // singleton
  ROS2Carla() {};

  

  std::shared_ptr<CarlaEgoVehicleControlSubscriber> _controller;
  std::shared_ptr<CarlaClockPublisher> _clock_publisher;
  std::unordered_map<void *, std::shared_ptr<CarlaPublisher>> _publishers;
  std::unordered_map<void *, std::shared_ptr<CarlaTransformPublisher>> _transforms;
  std::unordered_set<carla::streaming::detail::stream_id_type> _publish_stream;
  std::unordered_map<void *, ActorCallback> _actor_callbacks;
#if defined(WITH_ROS2Carla_DEMO)
  std::shared_ptr<BasicSubscriber> _basic_subscriber;
  std::shared_ptr<BasicPublisher> _basic_publisher;
  std::unordered_map<void *, ActorMessageCallback> _actor_message_callbacks;
#endif
};

} // namespace ros2
} // namespace carla
