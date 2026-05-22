// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PointCloudFieldsLayout.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace carla {
namespace ros2 {

// Forward declarations keep the FastDDS-heavy PublisherImpl<> definition out
// of the main carla-server compile unit. The full template definition + the
// PointCloud2 Traits instantiation live in CarlaPointCloudPublisher.cpp,
// which is built by the carla-ros2-native ExternalProject (where FastDDS
// headers are on the include path). Same Pimpl pattern as PR-3's
// CarlaCameraPublisher.{h,cpp}.
template <typename Traits> class PublisherImpl;
struct CarlaPointCloudMsgTraits;

// Abstract base for ROS 2 publishers that ship sensor_msgs::msg::PointCloud2:
// lidar, semantic-lidar, radar, and the point-cloud side of the DVS sensor.
// Each subclass declares its PointField layout (via PointCloudFieldsLayout.h)
// and implements ComputePointCloud(), which is responsible for any per-point
// transform (handedness flip, polar-to-Cartesian conversion, etc.) and
// returns the bytes that get written to the wire. The Init() call on the
// underlying PublisherImpl happens in the ctor, so a subclass only has to
// pass the (base_topic_name, frame_id) pair through.
class CarlaPointCloudPublisher : public BasePublisher {
public:
  CarlaPointCloudPublisher(std::string base_topic_name, std::string frame_id);
  ~CarlaPointCloudPublisher() override;

  CarlaPointCloudPublisher(const CarlaPointCloudPublisher &) = delete;
  CarlaPointCloudPublisher &operator=(const CarlaPointCloudPublisher &) = delete;
  CarlaPointCloudPublisher(CarlaPointCloudPublisher &&) noexcept = default;
  CarlaPointCloudPublisher &operator=(CarlaPointCloudPublisher &&) noexcept = default;

  bool Publish() override;

  bool WritePointCloud(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      std::uint32_t height,
      std::uint32_t width,
      const std::uint8_t *data);

protected:
  [[nodiscard]] virtual std::size_t GetPointSize() const = 0;
  [[nodiscard]] virtual const PointFieldDescriptor *GetFieldDescriptors() const = 0;
  [[nodiscard]] virtual std::size_t GetFieldDescriptorCount() const = 0;
  [[nodiscard]] virtual std::vector<std::uint8_t> ComputePointCloud(
      std::uint32_t height, std::uint32_t width, const std::uint8_t *data) const = 0;

private:
  bool WritePointCloud(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      std::uint32_t height,
      std::uint32_t width,
      std::vector<std::uint8_t> data);

  std::shared_ptr<PublisherImpl<CarlaPointCloudMsgTraits>> _impl;
};

}  // namespace ros2
}  // namespace carla
