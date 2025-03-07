// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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


struct FActorDescription;

namespace carla {
namespace ros2 {

class ROS2;

class ROS2Interfaces
{
  public:

  // deleting copy constructor for singleton
  ROS2Interfaces(const ROS2Interfaces& obj) = delete;
  static std::shared_ptr<ROS2Interfaces> GetInstance() {
    if (!_instance)
      _instance = std::shared_ptr<ROS2Interfaces>(new ROS2Interfaces);
    return _instance;
  }

  // general
  void Enable(bool enabled);
  void Shutdown();
  void SetFrame(uint64_t frame);
  void SetTimestamp(double timestamp);
  void RegisterInterface(std::shared_ptr<ROS2> newInterface);
  void RegisterActorWithInterfaces(FActorDescription& Description, std::string RosName, void* Actor);
  void RemoveActorFromInterfaces(void* Actor);
  void UnregisterInterface(std::shared_ptr<ROS2> interfaceToRemove);
  void CleanExpiredInterfaces();

  // ros_name managing
  void AddActorRosName(void *actor, std::string ros_name);
  void AddActorParentRosName(void *actor, void* parent);
  void RemoveActorRosName(void *actor);
  void UpdateActorRosName(void *actor, std::string ros_name);
  std::string GetActorRosName(void *actor);
  std::string GetActorParentRosName(void *actor, bool chain_names=true);

  // singleton
  ROS2Interfaces() {};

  private:

  static std::shared_ptr<ROS2Interfaces> _instance;

  bool _enabled = false;
  std::vector<std::weak_ptr<ROS2>> _interfaces;

  std::unordered_map<void *, std::string> _actor_ros_name;
  std::unordered_map<void *, std::vector<void*> > _actor_parent_ros_name;

};

} // namespace ros2
} // namespace carla
