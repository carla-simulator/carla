// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

namespace carla {
namespace ros2 {

  struct CarlaCollisionPublisherImpl;

  class CarlaCollisionPublisher {
    public:
      CarlaCollisionPublisher();
      ~CarlaCollisionPublisher();
      CarlaCollisionPublisher(const CarlaCollisionPublisher&) = delete;
      CarlaCollisionPublisher& operator=(const CarlaCollisionPublisher&) = delete;
      CarlaCollisionPublisher(CarlaCollisionPublisher&&) = delete;
      CarlaCollisionPublisher& operator=(CarlaCollisionPublisher&&) = delete;

      bool Init();
      bool Publish();
      void SetData(uint32_t actor_id, const uint8_t* data, const char* frame_id);
      private:
      void SetData(uint32_t actor_id, std::vector<float>&& data, const char* frame_id);
      
    private:
      std::unique_ptr<CarlaCollisionPublisherImpl> _impl;
  };
}
}