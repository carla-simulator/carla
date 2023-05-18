// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

namespace carla {
namespace ros2 {

  struct CarlaRGBCameraPublisherImpl;

  class CarlaRGBCameraPublisher {
    public:
      CarlaRGBCameraPublisher();
      ~CarlaRGBCameraPublisher();
      CarlaRGBCameraPublisher(const CarlaRGBCameraPublisher&) = delete;
      CarlaRGBCameraPublisher& operator=(const CarlaRGBCameraPublisher&) = delete;
      CarlaRGBCameraPublisher(CarlaRGBCameraPublisher&&) = delete;
      CarlaRGBCameraPublisher& operator=(CarlaRGBCameraPublisher&&) = delete;

      bool Init();
      bool Publish();
      void SetData(size_t height, size_t width, const uint8_t* data, const char* frame_id);
      private:
      void SetData(size_t height, size_t width, std::vector<uint8_t>&& data, const char* frame_id);
      
    private:
      std::unique_ptr<CarlaRGBCameraPublisherImpl> _impl;
  };
}
}
