// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

namespace carla {
namespace ros2 {

  struct CarlaImagePublisherImpl;

  class CarlaImagePublisher
  {
    public:
        CarlaImagePublisher();
        ~CarlaImagePublisher();
        CarlaImagePublisher(const CarlaImagePublisher&) = delete;
        CarlaImagePublisher& operator=(const CarlaImagePublisher&) = delete;
        CarlaImagePublisher(CarlaImagePublisher&&) = delete;
        CarlaImagePublisher& operator=(CarlaImagePublisher&&) = delete;

        bool Init(const char* topic);
        bool Publish();

        void SetImage(uint32_t height, uint32_t width, const std::vector<uint8_t>& data);

    private:
        std::unique_ptr<CarlaImagePublisherImpl> m_Impl;
  };
}
}
