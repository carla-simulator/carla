// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  struct CarlaLineInvasionPublisherImpl;

  class CarlaLineInvasionPublisher : public CarlaPublisher {
    public:
      CarlaLineInvasionPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaLineInvasionPublisher();
      CarlaLineInvasionPublisher(const CarlaLineInvasionPublisher&);
      CarlaLineInvasionPublisher& operator=(const CarlaLineInvasionPublisher&);
      CarlaLineInvasionPublisher(CarlaLineInvasionPublisher&&);
      CarlaLineInvasionPublisher& operator=(CarlaLineInvasionPublisher&&);

      bool Init();
      bool Publish();
      void SetData(int32_t seconds, uint32_t nanoseconds, const int32_t* data);
      const char* type() const override { return "line invasion"; }

    private:
      std::shared_ptr<CarlaLineInvasionPublisherImpl> _impl;
  };
}
}
