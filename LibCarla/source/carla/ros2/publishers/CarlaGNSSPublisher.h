// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

namespace carla {
namespace ros2 {

  struct CarlaGNSSPublisherImpl;

  class CarlaGNSSPublisher {
    public:
      CarlaGNSSPublisher();
      ~CarlaGNSSPublisher();
      CarlaGNSSPublisher(const CarlaGNSSPublisher&) = delete;
      CarlaGNSSPublisher& operator=(const CarlaGNSSPublisher&) = delete;
      CarlaGNSSPublisher(CarlaGNSSPublisher&&) = delete;
      CarlaGNSSPublisher& operator=(CarlaGNSSPublisher&&) = delete;

      bool Init();
      bool Publish();
      void SetData(const double* data, const char* frame_id);
      
    private:
      std::unique_ptr<CarlaGNSSPublisherImpl> _impl;
  };
}
}