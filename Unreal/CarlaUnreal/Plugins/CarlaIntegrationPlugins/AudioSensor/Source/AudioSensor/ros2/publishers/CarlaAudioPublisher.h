// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/publishers/CarlaPublisher.h"

namespace carla {
namespace ros2 {

  struct CarlaAudioPublisherImpl;

  class CarlaAudioPublisher : public CarlaPublisher {
    public:
      CarlaAudioPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaAudioPublisher();
      CarlaAudioPublisher(const CarlaAudioPublisher&);
      CarlaAudioPublisher& operator=(const CarlaAudioPublisher&);
      CarlaAudioPublisher(CarlaAudioPublisher&&);
      CarlaAudioPublisher& operator=(CarlaAudioPublisher&&);

      bool Init();
      bool Publish();
      void SetData(int32_t seconds, uint32_t nanoseconds, uint32_t num_channels, uint32_t sample_rate, uint32_t chunk_size, const int16_t* data);
      const char* type() const override { return "audio"; }

    private:
      void SetData(int32_t seconds, uint32_t nanoseconds, uint32_t sample_format, uint32_t num_channels, uint32_t sample_rate, uint32_t chunk_size, std::vector<int16_t>&& data);

    private:
      std::shared_ptr<CarlaAudioPublisherImpl> _impl;
  };
}
}
