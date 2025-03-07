// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>

namespace carla {
namespace ros2 {

  class CarlaCameraControlSubscriberListenerImpl;
  class CarlaCameraControlSubscriber;

  class CarlaCameraControlSubscriberListener {
    public:
      CarlaCameraControlSubscriberListener(CarlaCameraControlSubscriber* owner);
      ~CarlaCameraControlSubscriberListener();
      CarlaCameraControlSubscriberListener(const CarlaCameraControlSubscriberListener&) = delete;
      CarlaCameraControlSubscriberListener& operator=(const CarlaCameraControlSubscriberListener&) = delete;
      CarlaCameraControlSubscriberListener(CarlaCameraControlSubscriberListener&&) = delete;
      CarlaCameraControlSubscriberListener& operator=(CarlaCameraControlSubscriberListener&&) = delete;

      void SetOwner(CarlaCameraControlSubscriber* owner);

      std::unique_ptr<CarlaCameraControlSubscriberListenerImpl> _impl;
  };
}
}
