// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>

namespace carla {
namespace ros2 {

  class CarlaVehicleSubscriberListenerImpl;
  class CarlaEgoVehicleControlSubscriber;

  class CarlaVehicleSubscriberListener {
    public:
      CarlaVehicleSubscriberListener(CarlaEgoVehicleControlSubscriber* owner);
      ~CarlaVehicleSubscriberListener();
      CarlaVehicleSubscriberListener(const CarlaVehicleSubscriberListener&) = delete;
      CarlaVehicleSubscriberListener& operator=(const CarlaVehicleSubscriberListener&) = delete;
      CarlaVehicleSubscriberListener(CarlaVehicleSubscriberListener&&) = delete;
      CarlaVehicleSubscriberListener& operator=(CarlaVehicleSubscriberListener&&) = delete;

      void SetOwner(CarlaEgoVehicleControlSubscriber* owner);

      std::unique_ptr<CarlaVehicleSubscriberListenerImpl> _impl;
  };
}
}
