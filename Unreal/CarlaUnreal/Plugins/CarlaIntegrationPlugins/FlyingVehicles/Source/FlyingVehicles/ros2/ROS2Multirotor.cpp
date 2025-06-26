// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ROS2Multirotor.h"
#include "carla/Logging.h"
#include "MultirotorROS2Handler.h"

#include "subscribers/CarlaMultirotorControlSubscriber.h"



namespace carla {
namespace ros2 {

std::shared_ptr<ROS2Multirotor> ROS2Multirotor::_instance;


void ROS2Multirotor::SetFrame(uint64_t frame) {
  _frame = frame;
   //log_info("ROS2 new frame: ", _frame);

   for (auto multirotorControllerPair : _multirotorControllers) {
    std::shared_ptr<CarlaMultirotorControlSubscriber> multirotorController = multirotorControllerPair.second;
    void* actor = multirotorController->GetMultirotor();
    if (multirotorController->IsAlive()) {
      if (multirotorController->HasNewMessage()) {
        auto it = _multirotor_actor_callbacks.find(actor);
        if (it != _multirotor_actor_callbacks.end()) {
          MultirotorControl control = multirotorController->GetMessage();
          it->second(actor, control);
        }
      }
    } else {
      RemoveMultirotorActorCallback(actor);
    }
   }
}

void ROS2Multirotor::RegisterActor(FActorDescription& Description, std::string RosName, void *Actor)
{
  
  for (auto &&Attr : Description.Variations)
  {
    if (Attr.Key == "control_type" && (Attr.Value.Value == "multirotor"))
    {
      AddMultirotorActorCallback(Actor, RosName, [RosName](void *Actor, carla::ros2::ROS2MultirotorCallbackData Data) -> void
      {
        AActor *UEActor = static_cast<AActor*>(Actor);
        MultirotorROS2Handler Handler(UEActor, RosName);
        std::visit(Handler, Data);
      });
    }
  }
}

void ROS2Multirotor::RemoveActor(void* Actor)
{
  RemoveActorCallback(Actor);
}

void ROS2Multirotor::AddMultirotorActorCallback(void* actor, std::string ros_name, MultirotorActorCallback callback) {
  _multirotor_actor_callbacks.insert({actor, std::move(callback)});

  auto newController = std::make_shared<CarlaMultirotorControlSubscriber>(actor, ros_name.c_str());
  _multirotorControllers.insert({actor, newController});
  newController->Init();
}

void ROS2Multirotor::RemoveActorCallback(void* actor){

  auto mIt = _multirotor_actor_callbacks.find(actor);
  if (mIt != _multirotor_actor_callbacks.end()) {
    RemoveMultirotorActorCallback(actor);
  }
}

void ROS2Multirotor::RemoveMultirotorActorCallback(void* actor) {
  _multirotorControllers.erase(actor);
  _multirotor_actor_callbacks.erase(actor);
}


void ROS2Multirotor::Shutdown() {
  _multirotorControllers.clear();
  _enabled = false;

}

} // namespace ros2
} // namespace carla
