// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "CarlaSubscriber.h"
#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

  struct BasicSubscriberImpl;

  class BasicSubscriber : public CarlaSubscriber {
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
      // bool Read();
      const char* type() const override { return "basic_subscriber"; }

      //Do not call, for internal use only
      void ForwardMessage(const std::string& message);
      void DestroySubscriber();

    private:
      std::shared_ptr<BasicSubscriberImpl> _impl;
  };
}
}
