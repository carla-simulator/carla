// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>

namespace carla {
namespace ros2 {

  class CarlaImageListenerImpl;

  class CarlaImageListener {
    public:
      CarlaImageListener();
      ~CarlaImageListener();
      CarlaImageListener(const CarlaImageListener&) = delete;
      CarlaImageListener& operator=(const CarlaImageListener&) = delete;
      CarlaImageListener(CarlaImageListener&&) = delete;
      CarlaImageListener& operator=(CarlaImageListener&&) = delete;

      std::unique_ptr<CarlaImageListenerImpl> _impl;
  };
}
}
