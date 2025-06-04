// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  struct CarlaTransformPublisherImpl;

  class CarlaTransformPublisher : public CarlaPublisher {
    public:
      CarlaTransformPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaTransformPublisher();
      CarlaTransformPublisher(const CarlaTransformPublisher&);
      CarlaTransformPublisher& operator=(const CarlaTransformPublisher&);
      CarlaTransformPublisher(CarlaTransformPublisher&&);
      CarlaTransformPublisher& operator=(CarlaTransformPublisher&&);

      bool Init();
      bool Publish();
      void SetData(int32_t seconds, uint32_t nanoseconds, const float* translation, const float* rotation);
      const char* type() const override { return "transform"; }

    private:
      std::shared_ptr<CarlaTransformPublisherImpl> _impl;
  };
}
}
