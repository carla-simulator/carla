// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDispatcher.h"
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

namespace carla {
namespace ros2 {

  class CarlaAudioPublisher;

class ROS2Audio : public ROS2
{
  public:

  // deleting copy constructor for singleton
  ROS2Audio(const ROS2Audio& obj) = delete;
  static std::shared_ptr<ROS2Audio> GetInstance() {
    if (!_instance)
    {
      _instance = std::shared_ptr<ROS2Audio>(new ROS2Audio);
      auto ROS2Interfaces = UActorDispatcher::GetInterfaces();
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

  // receiving data to publish
  void ProcessDataFromAudio(
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      uint32_t num_channels, uint32_t sample_rate,
      uint32_t chunk_size, const int16_t *data,
      void *actor = nullptr);

    // enabling streams to publish
  virtual void EnableStream(carla::streaming::detail::stream_id_type id) { _publish_stream.insert(id); }
  virtual bool IsStreamEnabled(carla::streaming::detail::stream_id_type id) { return _publish_stream.count(id) > 0; }
  virtual void ResetStreams() { _publish_stream.clear(); }

  static std::shared_ptr<ROS2Audio> _instance;

  private:
  std::pair<std::shared_ptr<CarlaAudioPublisher>, std::shared_ptr<CarlaTransformPublisher>> GetOrCreateSensor(carla::streaming::detail::stream_id_type id, void* actor);

  // singleton
  ROS2Audio() {};

  std::unordered_map<void *, std::shared_ptr<CarlaAudioPublisher>> _publishers;
  std::unordered_map<void *, std::shared_ptr<CarlaTransformPublisher>> _transforms;
  std::unordered_set<carla::streaming::detail::stream_id_type> _publish_stream;
};

} // namespace ros2
} // namespace carla
