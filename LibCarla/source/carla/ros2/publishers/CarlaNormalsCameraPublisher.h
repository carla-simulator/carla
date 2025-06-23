// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  struct CarlaNormalsCameraPublisherImpl;
  struct CarlaCameraInfoPublisherImpl;

  class CarlaNormalsCameraPublisher : public CarlaPublisher {
    public:
      CarlaNormalsCameraPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaNormalsCameraPublisher();
      CarlaNormalsCameraPublisher(const CarlaNormalsCameraPublisher&);
      CarlaNormalsCameraPublisher& operator=(const CarlaNormalsCameraPublisher&);
      CarlaNormalsCameraPublisher(CarlaNormalsCameraPublisher&&);
      CarlaNormalsCameraPublisher& operator=(CarlaNormalsCameraPublisher&&);

      bool Init();
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);
      bool Publish();

      bool HasBeenInitialized() const;
      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data);
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);

      const char* type() const override { return "normals camera"; }

    private:
      bool InitImage();
      bool InitInfo();
      bool PublishImage();
      bool PublishInfo();

      void SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);

    private:
      std::shared_ptr<CarlaNormalsCameraPublisherImpl> _impl;
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;
  };
}
}
