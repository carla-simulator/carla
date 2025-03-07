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
#include "carla/ros2/ROS2CallbackData.h"
#include "carla/streaming/detail/Types.h"

#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <vector>

struct FActorDescription;

namespace carla {
namespace ros2 {

class ROS2
{
  public:

  // deleting copy constructor for singleton
  ROS2(const ROS2& obj) = delete;
  static std::shared_ptr<ROS2> GetInstance() {
    if (!_instance)
    {
      _instance = std::shared_ptr<ROS2>(new ROS2);
      auto ROS2Interfaces = carla::ros2::ROS2Interfaces::GetInstance();
      ROS2Interfaces->RegisterInterface(_instance);
    }
    return _instance;
  }

  virtual ~ROS2() {};

  // general
  virtual void Enable(bool enable) { _enabled = enable; };
  virtual void Shutdown() { _enabled = false; };
  virtual bool IsEnabled() { return _enabled; };
  virtual void SetFrame(uint64_t frame) { _frame = frame; };
  virtual void SetTimestamp(double timestamp);
  virtual void RegisterActor(FActorDescription& Description, std::string RosName, void* Actor) {};
  virtual void RemoveActor(void* Actor) {};
  virtual void RemoveActorRosPublishers(void *actor) {};

  // singleton
  ROS2() {};

  static std::shared_ptr<ROS2> _instance;

  bool _enabled { false };
  uint64_t _frame { 0 };
  int32_t _seconds { 0 };
  uint32_t _nanoseconds { 0 };

};

} // namespace ros2
} // namespace carla
