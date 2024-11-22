// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

namespace carla {
namespace ros2 {

  class CarlaSubscriberListenerImpl;
  class CarlaEgoVehicleControlSubscriber;

  class CarlaSubscriberListener {
    public:
      CarlaSubscriberListener(CarlaEgoVehicleControlSubscriber* owner);
      ~CarlaSubscriberListener();
      CarlaSubscriberListener(const CarlaSubscriberListener&) = delete;
      CarlaSubscriberListener& operator=(const CarlaSubscriberListener&) = delete;
      CarlaSubscriberListener(CarlaSubscriberListener&&) = delete;
      CarlaSubscriberListener& operator=(CarlaSubscriberListener&&) = delete;

      void SetOwner(CarlaEgoVehicleControlSubscriber* owner);

      std::unique_ptr<CarlaSubscriberListenerImpl> _impl;
  };
}
}
