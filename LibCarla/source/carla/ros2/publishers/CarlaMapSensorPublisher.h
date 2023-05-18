// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

namespace carla {
namespace ros2 {

  struct CarlaMapSensorPublisherImpl;

  class CarlaMapSensorPublisher {
    public:
      CarlaMapSensorPublisher();
      ~CarlaMapSensorPublisher();
      CarlaMapSensorPublisher(const CarlaMapSensorPublisher&) = delete;
      CarlaMapSensorPublisher& operator=(const CarlaMapSensorPublisher&) = delete;
      CarlaMapSensorPublisher(CarlaMapSensorPublisher&&) = delete;
      CarlaMapSensorPublisher& operator=(CarlaMapSensorPublisher&&) = delete;

      bool Init();
      bool Publish();
      void SetData(const char* data);
      
    private:
      std::unique_ptr<CarlaMapSensorPublisherImpl> _impl;
  };
}
}