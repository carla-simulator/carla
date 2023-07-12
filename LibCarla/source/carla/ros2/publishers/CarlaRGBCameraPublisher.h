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

  struct CarlaRGBCameraPublisherImpl;
  struct CarlaCameraInfoPublisherImpl;

  class CarlaRGBCameraPublisher : public CarlaPublisher {
    public:
      CarlaRGBCameraPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaRGBCameraPublisher();
      CarlaRGBCameraPublisher(const CarlaRGBCameraPublisher&);
      CarlaRGBCameraPublisher& operator=(const CarlaRGBCameraPublisher&);
      CarlaRGBCameraPublisher(CarlaRGBCameraPublisher&&);
      CarlaRGBCameraPublisher& operator=(CarlaRGBCameraPublisher&&);

      bool Init();
      bool Publish();
      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data);
      void SetInfoData(const double* data_D, const double* data_K, const double* data_R, const double* data_P);
      void SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);
      void SetInfoStaticData(uint32_t height, uint32_t width, const char* distortion = "plumb_bob", uint32_t binning_x = 0, uint32_t binning_y = 0);
      const char* type() const override { return "rgb camera"; }

    private:
      bool InitImage();
      bool InitInfo();
      bool PublishImage();
      bool PublishInfo();

      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);

    private:
      std::shared_ptr<CarlaRGBCameraPublisherImpl> _impl;
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;
  };
}
}
