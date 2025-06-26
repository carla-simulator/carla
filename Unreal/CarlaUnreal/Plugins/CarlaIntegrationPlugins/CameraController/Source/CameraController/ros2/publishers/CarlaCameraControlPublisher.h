// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/ros2/publishers/CarlaPublisher.h"

namespace carla {
namespace ros2 {

  struct CarlaCameraControlPublisherImpl;

  class CarlaCameraControlPublisher : public CarlaPublisher {
    public:
      CarlaCameraControlPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaCameraControlPublisher();
      CarlaCameraControlPublisher(const CarlaCameraControlPublisher&);
      CarlaCameraControlPublisher& operator=(const CarlaCameraControlPublisher&);
      CarlaCameraControlPublisher(CarlaCameraControlPublisher&&);
      CarlaCameraControlPublisher& operator=(CarlaCameraControlPublisher&&);

      bool Init();
      bool Publish();
      void SetData(int32_t seconds, uint32_t nanoseconds, float pan, float tilt, float zoom);
      const char* type() const override { return "camera control"; }

    private:
      std::shared_ptr<CarlaCameraControlPublisherImpl> _impl;
  };
}
}
