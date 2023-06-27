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

  struct CarlaDepthCameraPublisherImpl;

  class CarlaDepthCameraPublisher : public CarlaPublisher {
    public:
      CarlaDepthCameraPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaDepthCameraPublisher();
      CarlaDepthCameraPublisher(const CarlaDepthCameraPublisher&);
      CarlaDepthCameraPublisher& operator=(const CarlaDepthCameraPublisher&);
      CarlaDepthCameraPublisher(CarlaDepthCameraPublisher&&);
      CarlaDepthCameraPublisher& operator=(CarlaDepthCameraPublisher&&);

      bool Init();
      bool Publish();
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data);
      const char* type() const override { return "depth camera"; }

    private:
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);

    private:
      std::shared_ptr<CarlaDepthCameraPublisherImpl> _impl;
  };
}
}
