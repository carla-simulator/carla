// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaCameraPublisher.h"

namespace carla {
namespace ros2 {

bool CarlaCameraPublisher::WriteCameraInfo(int32_t seconds, uint32_t nanoseconds, uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
  
  _impl_camera_info->GetMessage()->header().stamp().sec(seconds);
  _impl_camera_info->GetMessage()->header().stamp().nanosec(nanoseconds);
  _impl_camera_info->GetMessage()->header().frame_id(GetFrameId());

  // TODO: Esto esta en el init del CameroInfo.cpp
      // string m_distortion_model
    // m_distortion_model = "plumb_bob";

    // const double cx = static_cast<double>(m_width) / 2.0;
    // const double cy = static_cast<double>(m_height) / 2.0;
    // const double fx = static_cast<double>(m_width) / (2.0 * std::tan(fov) * M_PI / 360.0);
    // const double fy = fx;

    // m_d = { 0.0, 0.0, 0.0, 0.0, 0.0 };
    // m_k = {fx, 0.0, cx, 0.0, fy, cy, 0.0, 0.0, 1.0};
    // m_r = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    // m_p = {fx, 0.0, cx, 0.0, 0.0, fy, cy, 0.0, 0.0, 0.0, 1.0, 0.0};

    // m_binning_x = 0;
    // m_binning_y = 0;


  _impl_camera_info->GetMessage()->roi().x_offset(x_offset);
  _impl_camera_info->GetMessage()->roi().y_offset(y_offset);
  _impl_camera_info->GetMessage()->roi().height(height);
  _impl_camera_info->GetMessage()->roi().width(width);
  _impl_camera_info->GetMessage()->roi().do_rectify(do_rectify);

  return true;
}

bool CarlaCameraPublisher::WriteImage(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, const uint8_t* data) {
  _impl_image->GetMessage()->header().stamp().sec(seconds);
  _impl_image->GetMessage()->header().stamp().nanosec(nanoseconds);
  _impl_image->GetMessage()->header().frame_id(this->GetFrameId());

  _impl_image->GetMessage()->width(width);
  _impl_image->GetMessage()->height(height);
  _impl_image->GetMessage()->encoding("bgra8");
  _impl_image->GetMessage()->is_bigendian(0);
  _impl_image->GetMessage()->step(width * sizeof(uint8_t) * 4);

  // TODO: Revisit
  std::vector<uint8_t> vector_data;
  const size_t size = height * width * 4;
  vector_data.resize(size);
  std::memcpy(&vector_data[0], &data[0], size);

  _impl_image->GetMessage()->data(std::move(vector_data)); //https://github.com/eProsima/Fast-DDS/issues/2330

  return true;
}

}  // namespace ros2
}  // namespace carla
