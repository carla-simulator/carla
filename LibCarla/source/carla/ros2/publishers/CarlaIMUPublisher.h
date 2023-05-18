// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

namespace carla {
namespace ros2 {

  struct CarlaIMUPublisherImpl;

  class CarlaIMUPublisher {
    public:
      CarlaIMUPublisher();
      ~CarlaIMUPublisher();
      CarlaIMUPublisher(const CarlaIMUPublisher&) = delete;
      CarlaIMUPublisher& operator=(const CarlaIMUPublisher&) = delete;
      CarlaIMUPublisher(CarlaIMUPublisher&&) = delete;
      CarlaIMUPublisher& operator=(CarlaIMUPublisher&&) = delete;

      bool Init();
      bool Publish();
      void SetData(float* accelerometer, float* gyroscope, float compass, const char* frame_id);
      
    private:
      std::unique_ptr<CarlaIMUPublisherImpl> _impl;
  };
}
}