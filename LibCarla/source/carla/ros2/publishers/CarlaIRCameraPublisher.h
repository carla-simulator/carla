// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  struct CarlaIRCameraPublisherImpl;
  struct CarlaCameraInfoPublisherImpl;

  class CarlaIRCameraPublisher : public CarlaPublisher {
    public:
      CarlaIRCameraPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaIRCameraPublisher();
      CarlaIRCameraPublisher(const CarlaIRCameraPublisher&);
      CarlaIRCameraPublisher& operator=(const CarlaIRCameraPublisher&);
      CarlaIRCameraPublisher(CarlaIRCameraPublisher&&);
      CarlaIRCameraPublisher& operator=(CarlaIRCameraPublisher&&);

      bool Init();
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);
      bool Publish();

      bool HasBeenInitialized() const;
      float GetFov() const;
      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data);
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);
      const char* type() const override { return "IR Camera"; }

    private:
      bool InitImage();
      bool InitInfo();
      bool PublishImage();
      bool PublishInfo();

      void SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);

    private:
      float _fov;
      std::shared_ptr<CarlaIRCameraPublisherImpl> _impl;
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;
  };
}
}
