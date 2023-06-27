// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  struct CarlaDVSCameraPublisherImpl;

  class CarlaDVSCameraPublisher : public CarlaPublisher {
    public:
      CarlaDVSCameraPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaDVSCameraPublisher();
      CarlaDVSCameraPublisher(const CarlaDVSCameraPublisher&);
      CarlaDVSCameraPublisher& operator=(const CarlaDVSCameraPublisher&);
      CarlaDVSCameraPublisher(CarlaDVSCameraPublisher&&);
      CarlaDVSCameraPublisher& operator=(CarlaDVSCameraPublisher&&);

      bool Init();
      bool Publish();
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data);
      const char* type() const override { return "dvs camera"; }

    private:
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);

    private:
      std::shared_ptr<CarlaDVSCameraPublisherImpl> _impl;
  };
}
}
