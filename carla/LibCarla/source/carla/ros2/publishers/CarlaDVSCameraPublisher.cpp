// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaDVSCameraPublisher.h"

#include "carla/ros2/publishers/DvsEventEncoding.h"
#include "carla/sensor/data/DVSEvent.h"

#include <cstring>
#include <utility>

namespace carla {
namespace ros2 {

bool CarlaDVSImagePublisher::WriteImageFromEvents(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    std::uint32_t height,
    std::uint32_t width,
    const std::uint8_t *events,
    std::size_t event_count,
    std::size_t event_stride) {
  auto encoded = EncodeDvsEventsToBgr(height, width, events, event_count, event_stride);
  // CarlaCameraPublisher's public WriteImage takes a const uint8_t* and runs
  // it through ComputeImage; the default ComputeImage just copies the bytes.
  // For DVS we have already produced the dense BGR buffer, so route through
  // the standard `data` pointer pathway.
  return WriteImage(seconds, nanoseconds, height, width, encoded.data());
}

std::size_t CarlaDVSPointCloudPublisher::GetPointSize() const {
  return sizeof(sensor::data::DVSEvent);
}

const PointFieldDescriptor *CarlaDVSPointCloudPublisher::GetFieldDescriptors() const {
  return kDvsFields.data();
}

std::size_t CarlaDVSPointCloudPublisher::GetFieldDescriptorCount() const {
  return kDvsFields.size();
}

std::vector<std::uint8_t> CarlaDVSPointCloudPublisher::ComputePointCloud(
    std::uint32_t height, std::uint32_t width, const std::uint8_t *data) const {
  const std::size_t total_points =
      static_cast<std::size_t>(height) * static_cast<std::size_t>(width);
  // The int64 event timestamp is converted to a real double in the FLOAT64 `t`
  // slot inside the seam (see EncodeDvsEventsToPointCloud); a raw byte copy
  // would leave int64 bits in a field a subscriber decodes as a double.
  return EncodeDvsEventsToPointCloud(
      data, total_points, sizeof(sensor::data::DVSEvent));
}

CarlaDVSCameraPublisher::CarlaDVSCameraPublisher(
    std::string base_topic_name, std::string frame_id)
  : BasePublisher(base_topic_name, frame_id),
    _image_pub(std::make_shared<CarlaDVSImagePublisher>(base_topic_name, frame_id)),
    _point_cloud_pub(std::make_shared<CarlaDVSPointCloudPublisher>(
        std::move(base_topic_name), std::move(frame_id))) {}

CarlaDVSCameraPublisher::~CarlaDVSCameraPublisher() = default;

bool CarlaDVSCameraPublisher::Publish() {
  const bool image_ok = _image_pub->Publish();
  const bool pc_ok = _point_cloud_pub->Publish();
  return image_ok && pc_ok;
}

bool CarlaDVSCameraPublisher::WriteCameraInfo(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    std::uint32_t x_offset,
    std::uint32_t y_offset,
    std::uint32_t height,
    std::uint32_t width,
    float fov,
    bool do_rectify) {
  return _image_pub->WriteCameraInfo(
      seconds, nanoseconds, x_offset, y_offset, height, width, fov, do_rectify);
}

bool CarlaDVSCameraPublisher::WriteImage(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    std::uint32_t height,
    std::uint32_t width,
    std::size_t event_count,
    const std::uint8_t *events,
    std::size_t event_stride) {
  return _image_pub->WriteImageFromEvents(
      seconds, nanoseconds, height, width, events, event_count, event_stride);
}

bool CarlaDVSCameraPublisher::WritePointCloud(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    std::uint32_t height,
    std::uint32_t width,
    const std::uint8_t *data) {
  return _point_cloud_pub->WritePointCloud(seconds, nanoseconds, height, width, data);
}

}  // namespace ros2
}  // namespace carla
