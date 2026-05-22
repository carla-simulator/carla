// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaCameraPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/publishers/CameraIntrinsics.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/types/CameraInfo.h"
#include "carla/ros2/types/CameraInfoPubSubTypes.h"
#include "carla/ros2/types/Image.h"
#include "carla/ros2/types/ImagePubSubTypes.h"

#include <cmath>
#include <utility>

namespace carla {
namespace ros2 {

struct CarlaCameraImageMsgTraits {
  using msg_type = sensor_msgs::msg::Image;
  using msg_pubsub_type = sensor_msgs::msg::ImagePubSubType;
};

struct CarlaCameraInfoMsgTraits {
  using msg_type = sensor_msgs::msg::CameraInfo;
  using msg_pubsub_type = sensor_msgs::msg::CameraInfoPubSubType;
};

CarlaCameraPublisher::CarlaCameraPublisher(
    std::string base_topic_name, std::string frame_id)
  : BasePublisher(std::move(base_topic_name), std::move(frame_id)),
    _impl_image(std::make_shared<PublisherImpl<CarlaCameraImageMsgTraits>>()),
    _impl_camera_info(std::make_shared<PublisherImpl<CarlaCameraInfoMsgTraits>>()) {
  if (!_impl_image->Init(GetBaseTopicName() + "/image")) {
    log_error("CarlaCameraPublisher: failed to initialise image writer for", GetBaseTopicName());
  }
  if (!_impl_camera_info->Init(GetBaseTopicName() + "/camera_info")) {
    log_error("CarlaCameraPublisher: failed to initialise camera_info writer for", GetBaseTopicName());
  }
}

CarlaCameraPublisher::~CarlaCameraPublisher() = default;

bool CarlaCameraPublisher::Publish() {
  // Publish both writers unconditionally so a failure on one does not silence
  // the other. Image goes first to match the previous per-sensor publishers.
  const bool image_ok = _impl_image->Publish();
  const bool info_ok = _impl_camera_info->Publish();
  return image_ok && info_ok;
}

std::vector<uint8_t> CarlaCameraPublisher::ComputeImage(
    uint32_t height, uint32_t width, const uint8_t *data) const {
  const size_t size =
      static_cast<size_t>(height) * static_cast<size_t>(width) * GetChannels();
  return std::vector<uint8_t>(data, data + size);
}

bool CarlaCameraPublisher::WriteCameraInfo(
    int32_t seconds,
    uint32_t nanoseconds,
    uint32_t x_offset,
    uint32_t y_offset,
    uint32_t height,
    uint32_t width,
    float fov,
    bool do_rectify) {
  auto *message = _impl_camera_info->GetMessage();
  message->header().stamp().sec(seconds);
  message->header().stamp().nanosec(nanoseconds);
  message->header().frame_id(GetFrameId());

  const auto k = ComputeIntrinsics(width, height, fov);

  message->height(height);
  message->width(width);
  message->distortion_model("plumb_bob");
  message->D({0.0, 0.0, 0.0, 0.0, 0.0});
  message->k({k.fx, 0.0, k.cx, 0.0, k.fy, k.cy, 0.0, 0.0, 1.0});
  message->r({1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0});
  message->p({k.fx, 0.0, k.cx, 0.0, 0.0, k.fy, k.cy, 0.0, 0.0, 0.0, 1.0, 0.0});
  message->binning_x(0);
  message->binning_y(0);

  message->roi().x_offset(x_offset);
  message->roi().y_offset(y_offset);
  message->roi().height(height);
  message->roi().width(width);
  message->roi().do_rectify(do_rectify);

  return true;
}

bool CarlaCameraPublisher::WriteImage(
    int32_t seconds,
    uint32_t nanoseconds,
    uint32_t height,
    uint32_t width,
    const uint8_t *data) {
  return WriteImage(seconds, nanoseconds, height, width, ComputeImage(height, width, data));
}

bool CarlaCameraPublisher::WriteImage(
    int32_t seconds,
    uint32_t nanoseconds,
    uint32_t height,
    uint32_t width,
    std::vector<uint8_t> data) {
  auto *message = _impl_image->GetMessage();
  message->header().stamp().sec(seconds);
  message->header().stamp().nanosec(nanoseconds);
  message->header().frame_id(GetFrameId());

  message->width(width);
  message->height(height);
  message->encoding(GetEncoding());
  message->is_bigendian(0);
  message->step(width * GetChannels() * static_cast<uint32_t>(sizeof(uint8_t)));
  // https://github.com/eProsima/Fast-DDS/issues/2330
  message->data(std::move(data));

  return true;
}

}  // namespace ros2
}  // namespace carla
