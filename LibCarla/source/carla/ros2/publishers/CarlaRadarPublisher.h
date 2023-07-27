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

  struct CarlaRadarPublisherImpl;

  class CarlaRadarPublisher : public CarlaPublisher {
    public:
      CarlaRadarPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaRadarPublisher();
      CarlaRadarPublisher(const CarlaRadarPublisher&);
      CarlaRadarPublisher& operator=(const CarlaRadarPublisher&);
      CarlaRadarPublisher(CarlaRadarPublisher&&);
      CarlaRadarPublisher& operator=(CarlaRadarPublisher&&);

      bool Init();
      bool Publish();
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, size_t elements, const uint8_t* data);
      const char* type() const override { return "radar"; }

    private:
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, size_t elements, std::vector<uint8_t>&& data);

    private:
      std::shared_ptr<CarlaRadarPublisherImpl> _impl;
  };
}
}
