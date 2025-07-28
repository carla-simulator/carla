// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"

#include "carla/ros2/types/Image.h"
#include "carla/ros2/types/ImagePubSubTypes.h"
#include "carla/ros2/types/CameraInfo.h"
#include "carla/ros2/types/CameraInfoPubSubTypes.h"

namespace carla {
namespace ros2 {

  class CarlaCameraPublisher : public BasePublisher {
    public:
      struct ImageMsgTraits {
        using msg_type = sensor_msgs::msg::Image;
        using msg_pubsub_type = sensor_msgs::msg::ImagePubSubType;
      };

      struct CameraInfoMsgTraits {
        using msg_type = sensor_msgs::msg::CameraInfo;
        using msg_pubsub_type = sensor_msgs::msg::CameraInfoPubSubType;
      };

      CarlaCameraPublisher(std::string base_topic_name, std::string frame_id) :
        BasePublisher(base_topic_name, frame_id),
        _impl_image(std::make_shared<PublisherImpl<ImageMsgTraits>>()),
        _impl_camera_info(std::make_shared<PublisherImpl<CameraInfoMsgTraits>>()) {
          _impl_image->Init(GetBaseTopicName() + "/image");
          _impl_camera_info->Init(GetBaseTopicName() + "/camera_info");
        }

      virtual uint8_t GetChannels() = 0;

      bool Publish() {
        return _impl_camera_info->Publish() && _impl_image->Publish();
      }

      bool WriteCameraInfo(int32_t seconds, uint32_t nanoseconds, uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);
      bool WriteImage(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, const uint8_t* data) {
        return WriteImage(seconds, nanoseconds, height, width, ComputeImage(height, width, data));
      }
      bool WriteImage(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, std::vector<uint8_t> data);

    private:
      virtual std::string GetEncoding() = 0;

      virtual std::vector<uint8_t> ComputeImage(uint32_t height, uint32_t width, const uint8_t* data);

    private:
      std::shared_ptr<PublisherImpl<ImageMsgTraits>> _impl_image;
      std::shared_ptr<PublisherImpl<CameraInfoMsgTraits>> _impl_camera_info;
  };

}  // namespace ros2
}  // namespace carla
