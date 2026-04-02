// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "carla/ros2/publishers/BasePublisher.h"

#include "CarlaCameraPublisher.h"
#include "CarlaPointCloudPublisher.h"

#include "carla/sensor/data/DVSEvent.h"

namespace carla {
namespace ros2 {

  class CarlaDVSCameraPublisher : public CarlaCameraPublisher {
    public:
      CarlaDVSCameraPublisher(std::string base_topic_name, std::string frame_id):
        CarlaCameraPublisher(base_topic_name, frame_id) {}

      uint8_t GetChannels() override { return 3; }

    private:
      std::string GetEncoding() override { return "bgr8"; }
  };

  class CarlaDVSPointCloudPublisher : public CarlaPointCloudPublisher {
    public:
      CarlaDVSPointCloudPublisher(std::string base_topic_name, std::string frame_id):
        CarlaPointCloudPublisher(base_topic_name, frame_id) {}

    private:
      const size_t GetPointSize() override;
      std::vector<sensor_msgs::msg::PointField> GetFields() override;

      std::vector<uint8_t> ComputePointCloud(uint32_t height, uint32_t width, uint8_t *data) override;
  };

  class CarlaDVSPublisher : public BasePublisher {
    public:

      CarlaDVSPublisher(std::string base_topic_name, std::string frame_id) :
        BasePublisher(base_topic_name, frame_id) {
          _camera_pub = std::make_shared<CarlaDVSCameraPublisher>(base_topic_name, frame_id);
          _point_cloud_pub = std::make_shared<CarlaDVSPointCloudPublisher>(base_topic_name, frame_id);
      }

      bool Publish() {
        return _camera_pub->Publish() && _point_cloud_pub->Publish();
      }

      bool WriteCameraInfo(int32_t seconds, uint32_t nanoseconds, uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
        return _camera_pub->WriteCameraInfo(seconds, nanoseconds, x_offset, y_offset, height, width, fov, do_rectify);
      }
      bool WriteImage(int32_t seconds, uint32_t nanoseconds, uint32_t elements, uint32_t im_height, uint32_t im_width, const uint8_t *data) {
        const size_t im_size = im_width * im_height * _camera_pub->GetChannels();
        std::vector<uint8_t> im_data(im_size, 0);

        const carla::sensor::data::DVSEvent* events = reinterpret_cast<const carla::sensor::data::DVSEvent*>(data);
        for (size_t i = 0; i < elements; ++i) {
          const auto& event = events[i];
          size_t index = (event.y * im_width + event.x) * 3 + (static_cast<int>(event.pol) * 2);
          im_data[index] = 255;
        }

        return _camera_pub->WriteImage(seconds, nanoseconds, im_height, im_width, std::move(im_data));
      }
      bool WritePointCloud(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, uint8_t* data) {
        return _point_cloud_pub->WritePointCloud(seconds, nanoseconds, height, width, data);
      }

    private:
      std::shared_ptr<CarlaDVSCameraPublisher> _camera_pub;
      std::shared_ptr<CarlaDVSPointCloudPublisher> _point_cloud_pub;
  };

}  // namespace ros2
}  // namespace carla
