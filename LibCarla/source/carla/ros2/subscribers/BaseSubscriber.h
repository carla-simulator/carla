// Copyright (c) 2025Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
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

      BaseSubscriber() {}

      BaseSubscriber(std::string base_topic_name) :
        _base_topic_name(base_topic_name) {}

      BaseSubscriber(std::string base_topic_name, std::string frame_id) :
        _base_topic_name(base_topic_name),
        _frame_id(frame_id) {}

      BaseSubscriber(void* actor, std::string base_topic_name, std::string frame_id) :
        _actor(actor),
        _base_topic_name(base_topic_name),
        _frame_id(frame_id) {}

      virtual ~BaseSubscriber() = default;

      std::string GetBaseTopicName() { return _base_topic_name; }
      std::string GetFrameId() { return _frame_id; }

      virtual ROS2CallbackData GetMessage() = 0;
      virtual void ProcessMessages(ActorCallback callback) = 0;

      void* GetActor() { return _actor; }

    protected:
      void* _actor { nullptr };
      std::string _base_topic_name = "";
      std::string _frame_id = "";
  };

}  // namespace ros2
}  // namespace carla
