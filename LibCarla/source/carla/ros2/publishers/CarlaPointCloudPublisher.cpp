// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaPointCloudPublisher.h"

namespace carla {
namespace ros2 {

bool CarlaPointCloudPublisher::WritePointCloud(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, std::vector<uint8_t> data) {

  _impl->GetMessage()->header().stamp().sec(seconds);
  _impl->GetMessage()->header().stamp().nanosec(nanoseconds);
  _impl->GetMessage()->header().frame_id(GetFrameId());

  auto fields = GetFields();
  const size_t point_size = GetPointSize();

  _impl->GetMessage()->width(width);
  _impl->GetMessage()->height(height);
  _impl->GetMessage()->is_bigendian(false);
  _impl->GetMessage()->fields(fields);
  _impl->GetMessage()->point_step(point_size);
  _impl->GetMessage()->row_step(width * point_size);
  _impl->GetMessage()->is_dense(false); // True if there are not invalid points
  _impl->GetMessage()->data(std::move(data));

  return true;
}

}  // namespace ros2
}  // namespace carla
