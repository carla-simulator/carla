// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>

namespace carla {
namespace ros2 {

  class CarlaListenerImpl;

  class CarlaListener {
    public:
      CarlaListener();
      ~CarlaListener();
      CarlaListener(const CarlaListener&) = delete;
      CarlaListener& operator=(const CarlaListener&) = delete;
      CarlaListener(CarlaListener&&) = delete;
      CarlaListener& operator=(CarlaListener&&) = delete;

      std::unique_ptr<CarlaListenerImpl> _impl;
  };
}
}
