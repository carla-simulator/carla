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
#include "ROS2MultirotorCallbackData.h"
#include "carla/streaming/detail/Types.h"

#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <vector>


namespace carla {
namespace ros2 {

  class CarlaMultirotorControlSubscriber;

class ROS2Multirotor : public ROS2
{
  public:

  // deleting copy constructor for singleton
  ROS2Multirotor(const ROS2Multirotor& obj) = delete;

  static std::shared_ptr<ROS2Multirotor> GetInstance() {
    if (!_instance)
    {
      _instance = std::shared_ptr<ROS2Multirotor>(new ROS2Multirotor);
      auto ROS2Interfaces = UActorDispatcher::GetInterfaces();
      ROS2Interfaces->RegisterInterface(_instance);
    }
    return _instance;
  }

  // virtual void Enable(bool enable) override;
  virtual void Shutdown() override;
  // virtual bool IsEnabled() override { return _enabled; }
  virtual void SetFrame(uint64_t frame) override;
  // virtual void SetTimestamp(double timestamp) override;
  virtual void RegisterActor(FActorDescription& Description, std::string RosName, void* Actor) override;
  virtual void RemoveActor(void* Actor) override;

  void AddMultirotorActorCallback(void* actor, std::string ros_name, MultirotorActorCallback callback);
  void RemoveActorCallback(void* actor);
  void RemoveMultirotorActorCallback(void* actor);

  // UPROPERTY();
  static std::shared_ptr<ROS2Multirotor> _instance;

  private:

  // singleton
  ROS2Multirotor() {};

  std::unordered_map<void *, std::shared_ptr<CarlaMultirotorControlSubscriber>> _multirotorControllers;
  std::unordered_map<void *, MultirotorActorCallback> _multirotor_actor_callbacks;
};

} // namespace ros2
} // namespace carla
