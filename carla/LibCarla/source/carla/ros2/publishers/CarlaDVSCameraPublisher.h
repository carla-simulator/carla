// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/CarlaCameraPublisher.h"
#include "carla/ros2/publishers/CarlaPointCloudPublisher.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace carla {
namespace ros2 {

// Image-side companion publisher: writes the dense visualisation of the DVS
// event stream as a 3-channel BGR sensor_msgs/Image to the existing wire
// format ROS 2 subscribers depend on. The encoding (positive polarity =
// red, negative polarity = blue) lives in DvsEventEncoding.h so a GTest
// pins it without instantiating FastDDS.
class CarlaDVSImagePublisher final : public CarlaCameraPublisher {
public:
  CarlaDVSImagePublisher(std::string base_topic_name, std::string frame_id)
    : CarlaCameraPublisher(std::move(base_topic_name), std::move(frame_id)) {}

  // Encode-from-events overload: walks the DVSEvent stream and writes the
  // dense image via the seam.
  bool WriteImageFromEvents(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      std::uint32_t height,
      std::uint32_t width,
      const std::uint8_t *events,
      std::size_t event_count,
      std::size_t event_stride);

protected:
  [[nodiscard]] std::uint8_t GetChannels() const override { return 3u; }
  [[nodiscard]] std::string GetEncoding() const override { return "bgr8"; }
};

// Point-cloud-side companion publisher: emits the raw DVS events as a
// sensor_msgs/PointCloud2 with the (x, y, t, pol) field layout shared with
// PR-3's pre-existing ue5-dev wire format.
class CarlaDVSPointCloudPublisher final : public CarlaPointCloudPublisher {
public:
  CarlaDVSPointCloudPublisher(std::string base_topic_name, std::string frame_id)
    : CarlaPointCloudPublisher(std::move(base_topic_name), std::move(frame_id)) {}

private:
  [[nodiscard]] std::size_t GetPointSize() const override;
  [[nodiscard]] const PointFieldDescriptor *GetFieldDescriptors() const override;
  [[nodiscard]] std::size_t GetFieldDescriptorCount() const override;
  [[nodiscard]] std::vector<std::uint8_t> ComputePointCloud(
      std::uint32_t height, std::uint32_t width, const std::uint8_t *data) const override;
};

// Public composite. Each tick the simulator hands the raw DVS event buffer
// once; the composite fans the data out to the image-side (encoded BGR
// visualisation) and the point-cloud-side (raw events) publishers.
class CarlaDVSCameraPublisher : public BasePublisher {
public:
  CarlaDVSCameraPublisher(std::string base_topic_name, std::string frame_id);
  ~CarlaDVSCameraPublisher() override;

  CarlaDVSCameraPublisher(const CarlaDVSCameraPublisher &) = delete;
  CarlaDVSCameraPublisher &operator=(const CarlaDVSCameraPublisher &) = delete;
  CarlaDVSCameraPublisher(CarlaDVSCameraPublisher &&) noexcept = default;
  CarlaDVSCameraPublisher &operator=(CarlaDVSCameraPublisher &&) noexcept = default;

  bool Publish() override;

  bool WriteCameraInfo(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      std::uint32_t x_offset,
      std::uint32_t y_offset,
      std::uint32_t height,
      std::uint32_t width,
      float fov,
      bool do_rectify);

  bool WriteImage(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      std::uint32_t height,
      std::uint32_t width,
      std::size_t event_count,
      const std::uint8_t *events,
      std::size_t event_stride);

  bool WritePointCloud(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      std::uint32_t height,
      std::uint32_t width,
      const std::uint8_t *data);

private:
  std::shared_ptr<CarlaDVSImagePublisher> _image_pub;
  std::shared_ptr<CarlaDVSPointCloudPublisher> _point_cloud_pub;
};

}  // namespace ros2
}  // namespace carla
