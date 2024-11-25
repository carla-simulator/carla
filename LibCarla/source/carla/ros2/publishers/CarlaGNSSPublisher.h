// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  struct CarlaGNSSPublisherImpl;

  class CarlaGNSSPublisher : public CarlaPublisher {
    public:
      CarlaGNSSPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaGNSSPublisher();
      CarlaGNSSPublisher(const CarlaGNSSPublisher&);
      CarlaGNSSPublisher& operator=(const CarlaGNSSPublisher&);
      CarlaGNSSPublisher(CarlaGNSSPublisher&&);
      CarlaGNSSPublisher& operator=(CarlaGNSSPublisher&&);

      bool Init();
      bool Publish();
      void SetData(int32_t seconds, uint32_t nanoseconds, const double* data);
      const char* type() const override { return "gnss"; }

    private:
      std::shared_ptr<CarlaGNSSPublisherImpl> _impl;
  };
}
}
