// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Logging.h"
#include <carla/ros2/ROS2Interfaces.h>
#include "carla/ros2/ROS2.h"
#include "carla/geom/GeoLocation.h"
#include "carla/geom/Vector3D.h"
#include "carla/sensor/data/DVSEvent.h"
#include "carla/sensor/data/LidarData.h"
#include "carla/sensor/data/SemanticLidarData.h"
#include "carla/sensor/data/RadarData.h"
#include "carla/sensor/data/Image.h"
#include "carla/sensor/s11n/ImageSerializer.h"
#include "carla/sensor/s11n/SensorHeaderSerializer.h"

#include "publishers/CarlaPublisher.h"
#include "publishers/CarlaClockPublisher.h"
#include "publishers/CarlaRGBCameraPublisher.h"
#include "publishers/CarlaDepthCameraPublisher.h"
#include "publishers/CarlaNormalsCameraPublisher.h"
#include "publishers/CarlaOpticalFlowCameraPublisher.h"
#include "publishers/CarlaSSCameraPublisher.h"
#include "publishers/CarlaISCameraPublisher.h"
#include "publishers/CarlaDVSCameraPublisher.h"
#include "publishers/CarlaLidarPublisher.h"
#include "publishers/CarlaSemanticLidarPublisher.h"
#include "publishers/CarlaRadarPublisher.h"
#include "publishers/CarlaIMUPublisher.h"
#include "publishers/CarlaGNSSPublisher.h"
#include "publishers/CarlaMapSensorPublisher.h"
#include "publishers/CarlaSpeedometerSensor.h"
#include "publishers/CarlaTransformPublisher.h"
#include "publishers/CarlaCollisionPublisher.h"
#include "publishers/CarlaLineInvasionPublisher.h"
#include "publishers/BasicPublisher.h"

#include "subscribers/CarlaSubscriber.h"
#include "subscribers/CarlaEgoVehicleControlSubscriber.h"
#if defined(WITH_ROS2_DEMO)
  #include "subscribers/BasicSubscriber.h"
#endif

#include <vector>

namespace carla {
namespace ros2 {

// static fields
std::shared_ptr<ROS2Interfaces> ROS2Interfaces::_instance = nullptr;

void ROS2Interfaces::Enable(bool enabled)
{
  bool needToCleanInterfaces = false;
  for (std::weak_ptr<ROS2> interfaceWeakPtr : _interfaces)
  {
    if (std::shared_ptr<ROS2> interfacePtr = interfaceWeakPtr.lock())
    {
      interfacePtr->Enable(enabled);
    }
    else
    {
      needToCleanInterfaces = true;
    }
    
  }
  if (needToCleanInterfaces)
  {
    CleanExpiredInterfaces();
  }
}

void ROS2Interfaces::Shutdown(){
  bool needToCleanInterfaces = false;
  for (std::weak_ptr<ROS2> interfaceWeakPtr : _interfaces)
  {
    if (std::shared_ptr<ROS2> interfacePtr = interfaceWeakPtr.lock())
    {
      if (interfacePtr->IsEnabled())
        interfacePtr->Shutdown();
    }
    else
    {
      needToCleanInterfaces = true;
    }
  }

  if (needToCleanInterfaces)
  {
    CleanExpiredInterfaces();
  }
    
}

void ROS2Interfaces::SetFrame(uint64_t frame)
{
  bool needToCleanInterfaces = false;
  for (std::weak_ptr<ROS2> interfaceWeakPtr : _interfaces)
  {
    if (std::shared_ptr<ROS2> interfacePtr = interfaceWeakPtr.lock())
    {
      interfacePtr->SetFrame(frame);
    }
    else
    {
      needToCleanInterfaces = true;
    }
  }
  if (needToCleanInterfaces)
  {
    CleanExpiredInterfaces();
  }
}

void ROS2Interfaces::SetTimestamp(double timestamp)
{
  bool needToCleanInterfaces = false;
  for (std::weak_ptr<ROS2> interfaceWeakPtr : _interfaces)
  {
    if (std::shared_ptr<ROS2> interfacePtr = interfaceWeakPtr.lock())
    {
      interfacePtr->SetTimestamp(timestamp);
    }
    else
    {
      needToCleanInterfaces = true;
    }
  }
  if (needToCleanInterfaces)
  {
    CleanExpiredInterfaces();
  }
}

void ROS2Interfaces::RegisterActorWithInterfaces(FActorDescription& Description, std::string RosName, void* Actor)
{
  bool needToCleanInterfaces = false;
  for (std::weak_ptr<ROS2> interfaceWeakPtr : _interfaces)
  {
    if (std::shared_ptr<ROS2> interfacePtr = interfaceWeakPtr.lock())
    {
      interfacePtr->RegisterActor(Description, RosName, Actor);
    }
    else
    {
      needToCleanInterfaces = true;
    }
  }
  if (needToCleanInterfaces)
  {
    CleanExpiredInterfaces();
  }
}

void ROS2Interfaces::RemoveActorFromInterfaces(void* Actor)
{
  bool needToCleanInterfaces = false;
  for (std::weak_ptr<ROS2> interfaceWeakPtr : _interfaces)
  {
    if (std::shared_ptr<ROS2> interfacePtr = interfaceWeakPtr.lock())
    {
      interfacePtr->RemoveActor(Actor);
    }
    else
    {
      needToCleanInterfaces = true;
    }
  }
  if (needToCleanInterfaces)
  {
    CleanExpiredInterfaces();
  }
}

void ROS2Interfaces::RegisterInterface(std::shared_ptr<ROS2> newInterface)
{
  _interfaces.push_back(newInterface);
}

void ROS2Interfaces::CleanExpiredInterfaces()
{
  _interfaces.erase(std::remove_if(_interfaces.begin(), _interfaces.end(),
    [](const std::weak_ptr<ROS2>& weakPtr) {
        return weakPtr.expired();
    }),
    _interfaces.end());
}

void ROS2Interfaces::UnregisterInterface(std::shared_ptr<ROS2> interfaceToRemove)
{
  _interfaces.erase(std::remove_if(_interfaces.begin(), _interfaces.end(),
        [&interfaceToRemove](const std::weak_ptr<ROS2>& weakPtr) {
            if (auto sp = weakPtr.lock()) {
                return sp == interfaceToRemove;
            }
            return false; // Skip expired weak_ptrs
        }),
        _interfaces.end());
}

void ROS2Interfaces::AddActorRosName(void *actor, std::string ros_name) {
  _actor_ros_name.insert({actor, ros_name});
}

void ROS2Interfaces::AddActorParentRosName(void *actor, void* parent) {
  auto it = _actor_parent_ros_name.find(actor);
  if (it != _actor_parent_ros_name.end()) {
    it->second.push_back(parent);
  } else {
    _actor_parent_ros_name.insert({actor, {parent}});
  }
}

void ROS2Interfaces::RemoveActorRosName(void *actor) {
  _actor_ros_name.erase(actor);
  _actor_parent_ros_name.erase(actor);

  bool needToCleanInterfaces = false;
  for (std::weak_ptr<ROS2> interfaceWeakPtr : _interfaces)
  {
    if (std::shared_ptr<ROS2> interfacePtr = interfaceWeakPtr.lock())
    {
      interfacePtr->RemoveActorRosPublishers(actor);
    }
    else
    {
      needToCleanInterfaces = true;
    }
  }
  if (needToCleanInterfaces)
  {
    CleanExpiredInterfaces();
  }
}

void ROS2Interfaces::UpdateActorRosName(void *actor, std::string ros_name) {
  auto it = _actor_ros_name.find(actor);
  if (it != _actor_ros_name.end()) {
    it->second = ros_name;
  }
}

std::string ROS2Interfaces::GetActorRosName(void *actor) {
  auto it = _actor_ros_name.find(actor);
  if (it != _actor_ros_name.end()) {
    return it->second;
  } else {
    return std::string("");
  }
}

std::string ROS2Interfaces::GetActorParentRosName(void *actor, bool chain_names) {
  auto it = _actor_parent_ros_name.find(actor);
  if (it != _actor_parent_ros_name.end())
  {
    const std::string current_actor_name = GetActorRosName(actor);
    std::string parent_name;
    for (auto parent_it = it->second.cbegin(); parent_it != it->second.cend(); ++parent_it)
    {
      const std::string name = GetActorRosName(*parent_it);
      if (name == current_actor_name)
      {
        continue;
      }
      if (name.empty())
      {
        continue;
      }
      if (!chain_names)
      {
        return name;
      }
      parent_name = name + '/' + parent_name;
    }
    if (parent_name.back() == '/')
      parent_name.pop_back();
    return parent_name;
  }
  else
    return std::string("");
}

} // namespace ros2
} // namespace carla
