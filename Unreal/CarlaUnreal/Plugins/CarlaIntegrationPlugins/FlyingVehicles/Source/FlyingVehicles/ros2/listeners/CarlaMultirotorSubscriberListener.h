// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

namespace carla {
namespace ros2 {

  class CarlaMultirotorSubscriberListenerImpl;
  class CarlaMultirotorControlSubscriber;

  class CarlaMultirotorSubscriberListener {
    public:
      CarlaMultirotorSubscriberListener(CarlaMultirotorControlSubscriber* owner);
      ~CarlaMultirotorSubscriberListener();
      CarlaMultirotorSubscriberListener(const CarlaMultirotorSubscriberListener&) = delete;
      CarlaMultirotorSubscriberListener& operator=(const CarlaMultirotorSubscriberListener&) = delete;
      CarlaMultirotorSubscriberListener(CarlaMultirotorSubscriberListener&&) = delete;
      CarlaMultirotorSubscriberListener& operator=(CarlaMultirotorSubscriberListener&&) = delete;

      void SetOwner(CarlaMultirotorControlSubscriber* owner);

      std::unique_ptr<CarlaMultirotorSubscriberListenerImpl> _impl;
  };
}
}
