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

  struct CarlaSpeedometerSensorImpl;

  class CarlaSpeedometerSensor : public CarlaPublisher {
    public:
      CarlaSpeedometerSensor(const char* ros_name = "", const char* parent = "");
      ~CarlaSpeedometerSensor();
      CarlaSpeedometerSensor(const CarlaSpeedometerSensor&);
      CarlaSpeedometerSensor& operator=(const CarlaSpeedometerSensor&);
      CarlaSpeedometerSensor(CarlaSpeedometerSensor&&);
      CarlaSpeedometerSensor& operator=(CarlaSpeedometerSensor&&);

      bool Init();
      bool Publish();
      void SetData(float data);
      const char* type() const override { return "speedometer"; }

    private:
      std::shared_ptr<CarlaSpeedometerSensorImpl> _impl;
  };
}
}
