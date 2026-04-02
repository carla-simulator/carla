// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

  struct BasicSubscriberImpl;

  class BasicSubscriber {
    public:
      BasicSubscriber(void* actor, const char* ros_name = "", const char* parent = "");
      ~BasicSubscriber();
      BasicSubscriber(const BasicSubscriber&);
      BasicSubscriber& operator=(const BasicSubscriber&);
      BasicSubscriber(BasicSubscriber&&);
      BasicSubscriber& operator=(BasicSubscriber&&);

      bool HasNewMessage();
      bool IsAlive();
      const char* GetMessage();
      void* GetActor();

      bool Init();
      const char* type() const { return "basic_subscriber"; }

      //Do not call, for internal use only
      void ForwardMessage(const std::string& message);
      void DestroySubscriber();

    protected:
      std::string _frame_id = "";
      std::string _name = "";
      std::string _parent = "";

    private:
      std::shared_ptr<BasicSubscriberImpl> _impl;
  };
}
}
