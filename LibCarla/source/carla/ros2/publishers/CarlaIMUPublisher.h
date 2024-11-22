// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  struct CarlaIMUPublisherImpl;

  class CarlaIMUPublisher : public CarlaPublisher {
    public:
      CarlaIMUPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaIMUPublisher();
      CarlaIMUPublisher(const CarlaIMUPublisher&);
      CarlaIMUPublisher& operator=(const CarlaIMUPublisher&);
      CarlaIMUPublisher(CarlaIMUPublisher&&);
      CarlaIMUPublisher& operator=(CarlaIMUPublisher&&);

      bool Init();
      bool Publish();
      void SetData(int32_t seconds, uint32_t nanoseconds, float* accelerometer, float* gyroscope, float compass);
      const char* type() const override { return "inertial measurement unit"; }

    private:
      std::shared_ptr<CarlaIMUPublisherImpl> _impl;
  };
}
}
