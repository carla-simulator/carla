// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>

#include "carla/ros2/ROS2CallbackData.h"


namespace carla {
namespace ros2 {

  class BaseSubscriber {
    public:
      
      BaseSubscriber(void* actor, const char* ros_name, const char* parent) :
        _actor(actor),
        _name(ros_name),
        _parent(parent) {}

      const std::string& frame_id() const { return _frame_id; }
      const std::string& name() const { return _name; }
      const std::string& parent() const { return _parent; }

      void frame_id(std::string&& frame_id) { _frame_id = std::move(frame_id); }
      void name(std::string&& name) { _name = std::move(name); }
      void parent(std::string&& parent) { _parent = std::move(parent); }

      std::string GetTopicName() {
        const std::string subscriber_type = this->GetSubscriberType();

        // If name is a complete topic name, use it
        if (subscriber_type.rfind("rt/") == 0) {
          return subscriber_type;
        } else {
          const std::string base { "rt/carla/" };
          std::string topic_name = base;
          if (!_parent.empty())
            topic_name += _parent + "/";
          topic_name += _name;
          topic_name += subscriber_type;
          return topic_name;
        }
      }
  
      virtual std::string GetSubscriberType() = 0;

      void* GetActor() { return _actor; }

      virtual ROS2CallbackData GetMessage() = 0;
      virtual void ProcessMessages(ActorCallback callback) = 0;

    protected:
      std::string _frame_id = "";
      std::string _name = "";
      std::string _parent = "";

      void* _actor { nullptr };
  };

}  // namespace ros2
}  // namespace carla
