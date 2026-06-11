// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  struct CarlaOdometryPublisherImpl;

  class CarlaOdometryPublisher : public CarlaPublisher {
    public:
      CarlaOdometryPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaOdometryPublisher();
      CarlaOdometryPublisher(const CarlaOdometryPublisher&);
      CarlaOdometryPublisher& operator=(const CarlaOdometryPublisher&);
      CarlaOdometryPublisher(CarlaOdometryPublisher&&);
      CarlaOdometryPublisher& operator=(CarlaOdometryPublisher&&);

      bool Init();
      bool Publish();
      void SetData(
          int32_t seconds,
          uint32_t nanoseconds,
          const float* location,
          const float* rotation,
          const float* linear_velocity,
          const float* angular_velocity);
      const char* type() const override { return "odometry"; }

    private:
      std::shared_ptr<CarlaOdometryPublisherImpl> _impl;
  };
}
}
