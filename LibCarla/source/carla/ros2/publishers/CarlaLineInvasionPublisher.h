// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

namespace carla {
namespace ros2 {

  struct CarlaLineInvasionPublisherImpl;

  class CarlaLineInvasionPublisher {
    public:
      CarlaLineInvasionPublisher();
      ~CarlaLineInvasionPublisher();
      CarlaLineInvasionPublisher(const CarlaLineInvasionPublisher&) = delete;
      CarlaLineInvasionPublisher& operator=(const CarlaLineInvasionPublisher&) = delete;
      CarlaLineInvasionPublisher(CarlaLineInvasionPublisher&&) = delete;
      CarlaLineInvasionPublisher& operator=(CarlaLineInvasionPublisher&&) = delete;

      bool Init();
      bool Publish();
      void SetData(int32_t* data, const char* frame_id);
      
    private:
      std::unique_ptr<CarlaLineInvasionPublisherImpl> _impl;
  };
}
}