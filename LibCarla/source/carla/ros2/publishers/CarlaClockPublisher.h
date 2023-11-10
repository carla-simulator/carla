// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  struct CarlaClockPublisherImpl;

  class CarlaClockPublisher : public CarlaPublisher {
    public:
      CarlaClockPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaClockPublisher();
      CarlaClockPublisher(const CarlaClockPublisher&);
      CarlaClockPublisher& operator=(const CarlaClockPublisher&);
      CarlaClockPublisher(CarlaClockPublisher&&);
      CarlaClockPublisher& operator=(CarlaClockPublisher&&);

      bool Init();
      bool Publish();
      void SetData(int32_t sec, uint32_t nanosec);
      const char* type() const override { return "clock"; }

    private:
      std::shared_ptr<CarlaClockPublisherImpl> _impl;
  };
}
}
