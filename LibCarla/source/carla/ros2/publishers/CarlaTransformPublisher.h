// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

namespace carla {
namespace ros2 {

  struct CarlaTransformPublisherImpl;

  class CarlaTransformPublisher {
    public:
      CarlaTransformPublisher();
      ~CarlaTransformPublisher();
      CarlaTransformPublisher(const CarlaTransformPublisher&) = delete;
      CarlaTransformPublisher& operator=(const CarlaTransformPublisher&) = delete;
      CarlaTransformPublisher(CarlaTransformPublisher&&) = delete;
      CarlaTransformPublisher& operator=(CarlaTransformPublisher&&) = delete;

      bool Init();
      bool Publish();
      void SetData(const float* translation, const float* rotation, const char* frame_id);
      
    private:
      std::unique_ptr<CarlaTransformPublisherImpl> _impl;
  };
}
}