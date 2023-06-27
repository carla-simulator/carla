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

  struct CarlaSSCameraPublisherImpl;

  class CarlaSSCameraPublisher : public CarlaPublisher {
    public:
      CarlaSSCameraPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaSSCameraPublisher();
      CarlaSSCameraPublisher(const CarlaSSCameraPublisher&);
      CarlaSSCameraPublisher& operator=(const CarlaSSCameraPublisher&);
      CarlaSSCameraPublisher(CarlaSSCameraPublisher&&);
      CarlaSSCameraPublisher& operator=(CarlaSSCameraPublisher&&);

      bool Init();
      bool Publish();
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data);
      const char* type() const override { return "semantic segmentation"; }

    private:
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);

    private:
      std::shared_ptr<CarlaSSCameraPublisherImpl> _impl;
  };
}
}
