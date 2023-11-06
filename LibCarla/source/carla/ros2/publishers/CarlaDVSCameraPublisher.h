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
  struct CarlaCameraInfoPublisherImpl;
  struct CarlaPointCloudPublisherImpl;

  class CarlaDVSCameraPublisher : public CarlaPublisher {
    public:
      CarlaDVSCameraPublisher(const char* ros_name = "", const char* parent = "");
      ~CarlaDVSCameraPublisher();
      CarlaDVSCameraPublisher(const CarlaDVSCameraPublisher&);
      CarlaDVSCameraPublisher& operator=(const CarlaDVSCameraPublisher&);
      CarlaDVSCameraPublisher(CarlaDVSCameraPublisher&&);
      CarlaDVSCameraPublisher& operator=(CarlaDVSCameraPublisher&&);

      bool Init();
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);
      bool Publish();

      bool HasBeenInitialized() const;
      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t elements, size_t height, size_t width, const uint8_t* data);
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);
      void SetPointCloudData(size_t height, size_t width, size_t elements, const uint8_t* data);
      const char* type() const override { return "dvs camera"; }

    private:
    private:
      bool InitImage();
      bool InitInfo();
      bool InitPointCloud();

      void SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);
      void SetPointCloudData(size_t height, size_t width, std::vector<uint8_t>&& data);
      bool PublishImage();
      bool PublishInfo();
      bool PublishPointCloud();

    private:
      std::shared_ptr<CarlaDVSCameraPublisherImpl> _impl;
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _info;
      std::shared_ptr<CarlaPointCloudPublisherImpl> _point_cloud;
  };
}
}
