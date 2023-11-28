// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

#include "CarlaSubscriber.h"
#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

  struct CarlaEgoVehicleControlSubscriberImpl;

  class CarlaEgoVehicleControlSubscriber : public CarlaSubscriber {
    public:
      CarlaEgoVehicleControlSubscriber(void* vehicle, const char* ros_name = "", const char* parent = "");
      ~CarlaEgoVehicleControlSubscriber();
      CarlaEgoVehicleControlSubscriber(const CarlaEgoVehicleControlSubscriber&);
      CarlaEgoVehicleControlSubscriber& operator=(const CarlaEgoVehicleControlSubscriber&);
      CarlaEgoVehicleControlSubscriber(CarlaEgoVehicleControlSubscriber&&);
      CarlaEgoVehicleControlSubscriber& operator=(CarlaEgoVehicleControlSubscriber&&);

      bool HasNewMessage();
      bool IsAlive();
      VehicleControl GetMessage();
      void* GetVehicle();

      bool Init();
      bool Read();
      const char* type() const override { return "Ego vehicle control"; }

      //Do not call, for internal use only
      void ForwardMessage(VehicleControl control);
      void DestroySubscriber();
    private:
      void SetData(int32_t seconds, uint32_t nanoseconds, uint32_t actor_id, std::vector<float>&& data);

    private:
      std::shared_ptr<CarlaEgoVehicleControlSubscriberImpl> _impl;
  };
}
}
