// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/CarlaSubscriber.h"
#include "FlyingVehicles/ros2/ROS2MultirotorCallbackData.h"

namespace carla {
namespace ros2 {

  struct CarlaMultirotorControlSubscriberImpl;

  class CarlaMultirotorControlSubscriber : public CarlaSubscriber {
    public:
      CarlaMultirotorControlSubscriber(void* multirotor, const char* ros_name = "", const char* parent = "");
      ~CarlaMultirotorControlSubscriber();
      CarlaMultirotorControlSubscriber(const CarlaMultirotorControlSubscriber&);
      CarlaMultirotorControlSubscriber& operator=(const CarlaMultirotorControlSubscriber&);
      CarlaMultirotorControlSubscriber(CarlaMultirotorControlSubscriber&&);
      CarlaMultirotorControlSubscriber& operator=(CarlaMultirotorControlSubscriber&&);

      bool HasNewMessage();
      bool IsAlive();
      MultirotorControl GetMessage();
      void* GetMultirotor();

      bool Init();
      bool Read();
      const char* type() const override { return "Multirotor control"; }

      //Do not call, for internal use only
      void ForwardMessage(MultirotorControl control);
      void DestroySubscriber();
    private:
      void SetData(int32_t seconds, uint32_t nanoseconds, uint32_t actor_id, std::vector<float>&& data);

    private:
      std::shared_ptr<CarlaMultirotorControlSubscriberImpl> _impl;
  };
}
}
