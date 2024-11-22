// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  struct CarlaMapSensorPublisherImpl;

  class CarlaMapSensorPublisher : public CarlaPublisher {
    public:
      CarlaMapSensorPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaMapSensorPublisher();
      CarlaMapSensorPublisher(const CarlaMapSensorPublisher&);
      CarlaMapSensorPublisher& operator=(const CarlaMapSensorPublisher&);
      CarlaMapSensorPublisher(CarlaMapSensorPublisher&&);
      CarlaMapSensorPublisher& operator=(CarlaMapSensorPublisher&&);

      bool Init();
      bool Publish();
      void SetData(const char* data);
      const char* type() const override { return "map sensor"; }

    private:
      std::shared_ptr<CarlaMapSensorPublisherImpl> _impl;
  };
}
}
