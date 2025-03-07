// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ROS2CameraControl.h"
#include "carla/Logging.h"

#include "carla/ros2/plugin-utils/enable-fastdds-include.h" // start fastdds includes
#include "carla/ros2/publishers/CarlaTransformPublisher.h"
#include "carla/ros2/subscribers/CarlaSubscriber.h"
#include "carla/ros2/plugin-utils/disable-fastdds-include.h" // end fastdds includes

#include <vector>

namespace carla {
namespace ros2 {

// static fields
std::shared_ptr<ROS2CameraControl> ROS2CameraControl::_instance;

void ROS2CameraControl::Enable(bool enable) {
  _enabled = enable;
  log_info("ROS2CameraControl enabled: ", _enabled);
}

void ROS2CameraControl::SetFrame(uint64_t frame) {
  _frame = frame;
   //log_info("ROS2CameraControl new frame: ", _frame);
   for (auto controllerPair : _controllers) {
    std::shared_ptr<CarlaCameraControlSubscriber> controller = controllerPair.second;
    void* actor = controller->GetCameraControl();
    if (controller->IsAlive()) {
      if (controller->HasNewMessage()) {
        auto it = _actor_callbacks.find(actor);
        if (it != _actor_callbacks.end()) {
          CameraControl control = controller->GetMessage();
          it->second(actor, control);
        }
      }
    } else {
      RemoveActorCallback(actor);
    }
   }
}

void ROS2CameraControl::SetTimestamp(double timestamp) {
  if (IsEnabled()){
    double integral;
    const double fractional = modf(timestamp, &integral);
    const double multiplier = 1000000000.0;
    _seconds = static_cast<int32_t>(integral);
    _nanoseconds = static_cast<uint32_t>(fractional * multiplier);
  }

}

void ROS2CameraControl::Shutdown() {
  for (auto& element : _publishers) {
    element.second.reset();
  }
  for (auto& element : _transforms) {
    element.second.reset();
  }
  _controllers.clear();
  _enabled = false;
}

void ROS2CameraControl::RegisterActor(FActorDescription& Description, std::string RosName, void *Actor)
{
  for (auto &&Attr : Description.Variations)
  {
    if (Attr.Key == "sensor_type" && (Attr.Value.Value == "camera_control"))
    {
      AddActorCallback(Actor, RosName, [RosName](void *Actor, carla::ros2::ROS2CameraControlCallbackData Data) -> void
      {
        AActor *UEActor = static_cast<AActor*>(Actor);
        CameraControlROS2Handler Handler(UEActor, RosName);
        std::visit(Handler, Data);
      });
    }
  }
}

void ROS2CameraControl::RemoveActor(void* Actor)
{
  RemoveActorCallback(Actor);
}

void ROS2CameraControl::RemoveActorRosPublishers(void *actor)
{
  auto p_it = _publishers.find(actor);
  if (p_it != _publishers.end()) {
    _publishers.erase(actor);
  }

  auto t_it = _transforms.find(actor);
  if (t_it != _transforms.end()) {
    _transforms.erase(actor);
  }
}

void ROS2CameraControl::AddActorCallback(void* actor, std::string ros_name, CameraControlActorCallback callback) {
  _actor_callbacks.insert({actor, std::move(callback)});
  auto ROS2Interfaces = UActorDispatcher::GetInterfaces();
  std::string parent_ros_name = ROS2Interfaces->GetActorParentRosName(actor);
  auto controller = std::make_shared<CarlaCameraControlSubscriber>(actor, ros_name.c_str(), parent_ros_name.c_str());
  _controllers.insert({actor, controller});
  controller->Init();
}

void ROS2CameraControl::RemoveActorCallback(void* actor) {
  auto it = _actor_callbacks.find(actor);
  if (it != _actor_callbacks.end()) {
    _controllers.erase(actor);
    _actor_callbacks.erase(actor);
  }
}

std::pair<std::shared_ptr<CarlaCameraControlPublisher>, std::shared_ptr<CarlaTransformPublisher>> ROS2CameraControl::GetOrCreateSensor(carla::streaming::detail::stream_id_type id, void* actor) {
  auto it_publishers = _publishers.find(actor);
  auto it_transforms = _transforms.find(actor);
  std::shared_ptr<CarlaCameraControlPublisher> publisher {};
  std::shared_ptr<CarlaTransformPublisher> transform {};
  auto ROS2Interfaces = UActorDispatcher::GetInterfaces();
  if (it_publishers != _publishers.end()) {
    publisher = it_publishers->second;
    if (it_transforms != _transforms.end()) {
      transform = it_transforms->second;
    }
  } else {
    //Sensor not found, creating one of the given type
    const std::string string_id = std::to_string(id);
    std::string ros_name = ROS2Interfaces->GetActorRosName(actor);
    std::string parent_ros_name = ROS2Interfaces->GetActorParentRosName(actor);
    std::string parent_ros_name_standalone = ROS2Interfaces->GetActorParentRosName(actor, false);
    if (ros_name == "camera_control__") {
      ros_name.pop_back();
      ros_name.pop_back();
      ros_name += string_id;
      ROS2Interfaces->UpdateActorRosName(actor, ros_name);
    }
    std::shared_ptr<CarlaCameraControlPublisher> new_publisher = std::make_shared<CarlaCameraControlPublisher>(ros_name.c_str(), parent_ros_name.c_str());
    if (new_publisher->Init()) {
      _publishers.insert({actor, new_publisher});
      publisher = new_publisher;
    }
    std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str(), parent_ros_name_standalone.c_str());
    if (new_transform->Init()) {
      _transforms.insert({actor, new_transform});
      transform = new_transform;
    }
  }
  return { publisher, transform };
}

void ROS2CameraControl::ProcessDataFromCameraControl(
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    float pan, float tilt, float zoom,
    void *actor) {
  log_info("Sensor CameraControl to ROS data: frame.", _frame, "stream.", stream_id, "camera_control.", pan, tilt, zoom);
  auto sensors = GetOrCreateSensor(stream_id, actor);
  if (sensors.first) {
    sensors.first->SetData(_seconds, _nanoseconds, pan, tilt, zoom);
    sensors.first->Publish();
  }
  if (sensors.second) {
    sensors.second->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    sensors.second->Publish();
  }
}

} // namespace ros2
} // namespace carla
