// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaCameraPublisher.h"

namespace carla {
namespace ros2 {

std::vector<uint8_t> CarlaCameraPublisher::ComputeImage(uint32_t height, uint32_t width, const uint8_t *data) {
  const size_t size = height * width * this->GetChannels() * sizeof(uint8_t);
  std::vector<uint8_t> vector_data(data, data + size);
  return vector_data;
}

bool CarlaCameraPublisher::WriteCameraInfo(int32_t seconds, uint32_t nanoseconds, uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
  
  _impl_camera_info->GetMessage()->header().stamp().sec(seconds);
  _impl_camera_info->GetMessage()->header().stamp().nanosec(nanoseconds);
  _impl_camera_info->GetMessage()->header().frame_id(GetFrameId());

  const double cx = static_cast<double>(width) / 2.0;
  const double cy = static_cast<double>(height) / 2.0;
  const double fx = static_cast<double>(width) / (2.0 * std::tan(fov) * M_PI / 360.0);
  const double fy = fx;

  _impl_camera_info->GetMessage()->height(height);
  _impl_camera_info->GetMessage()->width(width);
  _impl_camera_info->GetMessage()->distortion_model("plumb_bob");
  _impl_camera_info->GetMessage()->D({ 0.0, 0.0, 0.0, 0.0, 0.0 });
  _impl_camera_info->GetMessage()->k({fx, 0.0, cx, 0.0, fy, cy, 0.0, 0.0, 1.0});
  _impl_camera_info->GetMessage()->r({ 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 });
  _impl_camera_info->GetMessage()->p({fx, 0.0, cx, 0.0, 0.0, fy, cy, 0.0, 0.0, 0.0, 1.0, 0.0});
  _impl_camera_info->GetMessage()->binning_x(0);
  _impl_camera_info->GetMessage()->binning_y(0);

  _impl_camera_info->GetMessage()->roi().x_offset(x_offset);
  _impl_camera_info->GetMessage()->roi().y_offset(y_offset);
  _impl_camera_info->GetMessage()->roi().height(height);
  _impl_camera_info->GetMessage()->roi().width(width);
  _impl_camera_info->GetMessage()->roi().do_rectify(do_rectify);

  return true;
}

bool CarlaCameraPublisher::WriteImage(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, std::vector<uint8_t> data) {
  _impl_image->GetMessage()->header().stamp().sec(seconds);
  _impl_image->GetMessage()->header().stamp().nanosec(nanoseconds);
  _impl_image->GetMessage()->header().frame_id(this->GetFrameId());

  _impl_image->GetMessage()->width(width);
  _impl_image->GetMessage()->height(height);
  _impl_image->GetMessage()->encoding(this->GetEncoding());
  _impl_image->GetMessage()->is_bigendian(0);
  _impl_image->GetMessage()->step(width * this->GetChannels() * sizeof(uint8_t));

  _impl_image->GetMessage()->data(std::move(data)); // https://github.com/eProsima/Fast-DDS/issues/2330

  return true;
}

}  // namespace ros2
}  // namespace carla
