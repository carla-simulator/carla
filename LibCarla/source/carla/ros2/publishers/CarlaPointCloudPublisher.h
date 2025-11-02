// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"

#include "carla/ros2/types/PointCloud2.h"
#include "carla/ros2/types/PointCloud2PubSubTypes.h"

namespace carla {
namespace ros2 {

  class CarlaPointCloudPublisher : public BasePublisher {
    public:
      struct PointCloudMsgTraits {
        using msg_type = sensor_msgs::msg::PointCloud2;
        using msg_pubsub_type = sensor_msgs::msg::PointCloud2PubSubType;
      };

      CarlaPointCloudPublisher(std::string base_topic_name, std::string frame_id) :
        BasePublisher(base_topic_name, frame_id),
        _impl(std::make_shared<PublisherImpl<PointCloudMsgTraits>>()) {
          _impl->Init(GetBaseTopicName() + "/point_cloud");
        }

      bool Publish() {
        return _impl->Publish();
      }

      bool WritePointCloud(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, uint8_t* data) {
        return WritePointCloud(seconds, nanoseconds, height, width, ComputePointCloud(height, width, data));
      }
      bool WritePointCloud(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, std::vector<uint8_t> data);

    private:
      virtual const size_t GetPointSize() = 0;
      virtual std::vector<sensor_msgs::msg::PointField> GetFields() = 0;

      virtual std::vector<uint8_t> ComputePointCloud(uint32_t height, uint32_t width, uint8_t *data) = 0;


    private:
      std::shared_ptr<PublisherImpl<PointCloudMsgTraits>> _impl;
  };

}  // namespace ros2
}  // namespace carla
