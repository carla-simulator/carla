// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace carla {
namespace ros2 {

  class BasePublisher {
    public:

      BasePublisher() {}

      BasePublisher(std::string base_topic_name) :
        _base_topic_name(base_topic_name) {}

      BasePublisher(std::string base_topic_name, std::string frame_id) :
        _base_topic_name(base_topic_name),
        _frame_id(frame_id) {}

      BasePublisher(void* actor, std::string base_topic_name, std::string frame_id) :
        _actor(actor),
        _base_topic_name(base_topic_name),
        _frame_id(frame_id) {}

      const std::string GetBaseTopicName() {return _base_topic_name; }
      const std::string GetFrameId() { return _frame_id; }

      virtual bool Publish() = 0;

      void* GetActor() { return _actor; }

    protected:
      std::string _frame_id = "";
      std::string _base_topic_name = "";

      void* _actor { nullptr };
  };

}  // namespace ros2
}  // namespace carla
