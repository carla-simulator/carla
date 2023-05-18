// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

namespace carla {
namespace ros2 {

  struct CarlaSpeedometerSensorImpl;

  class CarlaSpeedometerSensor {
    public:
      CarlaSpeedometerSensor();
      ~CarlaSpeedometerSensor();
      CarlaSpeedometerSensor(const CarlaSpeedometerSensor&) = delete;
      CarlaSpeedometerSensor& operator=(const CarlaSpeedometerSensor&) = delete;
      CarlaSpeedometerSensor(CarlaSpeedometerSensor&&) = delete;
      CarlaSpeedometerSensor& operator=(CarlaSpeedometerSensor&&) = delete;

      bool Init();
      bool Publish();
      void SetData(float data);
      
    private:
      std::unique_ptr<CarlaSpeedometerSensorImpl> _impl;
  };
}
}