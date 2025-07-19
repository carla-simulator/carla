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

  class CarlaDVSPointCloudPublisher : public CarlaPointCloudPublisher {
    public:
      CarlaDVSPointCloudPublisher(std::string base_topic_name, std::string frame_id):
        CarlaPointCloudPublisher(base_topic_name, frame_id) {}

    private:
      const size_t GetPointSize() override;
      std::vector<sensor_msgs::msg::PointField> GetFields() override;

      std::vector<uint8_t> ComputePointCloud(uint32_t height, uint32_t width, float *data) override;
  };

  class CarlaDVSCameraPublisher : public BasePublisher {
    public:

      CarlaDVSCameraPublisher(std::string base_topic_name, std::string frame_id) :
        BasePublisher(base_topic_name, frame_id) {
          _camera_pub = std::make_shared<CarlaCameraPublisher>(base_topic_name, frame_id);
          _point_cloud_pub = std::make_shared<CarlaDVSPointCloudPublisher>(base_topic_name, frame_id);
      }

      bool Publish() {
        return _camera_pub->Publish() && _point_cloud_pub->Publish();
      }

      bool WriteCameraInfo(int32_t seconds, uint32_t nanoseconds, uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
        return _camera_pub->WriteCameraInfo(seconds, nanoseconds, x_offset, y_offset, height, width, fov, do_rectify);
      }
      bool WriteImage(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, const uint8_t* data) {
        // TODO: Inicializar todo a 0?
        std::vector<uint8_t> im_data;
        const size_t im_size = width * height * 3;
        im_data.resize(im_size);
        carla::sensor::data::DVSEvent* vec_event = (carla::sensor::data::DVSEvent*)&data[0];
        for (size_t i = 0; i < width; ++i, ++vec_event) {
          size_t index = (vec_event->y * width + vec_event->x) * 3 + (static_cast<int>(vec_event->pol) * 2);
          im_data[index] = 255;
        }
        return _camera_pub->WriteImage(seconds, nanoseconds, height, width, im_data.data());
      }
      bool WritePointCloud(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, float* data) {
        return _point_cloud_pub->WritePointCloud(seconds, nanoseconds, height, width, data);
      }

    private:
      std::shared_ptr<CarlaCameraPublisher> _camera_pub;
      std::shared_ptr<CarlaDVSPointCloudPublisher> _point_cloud_pub;
  };

}  // namespace ros2
}  // namespace carla
