// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaPointCloudPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/types/PointCloud2.h"
#include "carla/ros2/types/PointCloud2PubSubTypes.h"
#include "carla/ros2/types/PointField.h"

#include <string>
#include <utility>
#include <vector>

namespace carla {
namespace ros2 {

struct CarlaPointCloudMsgTraits {
  using msg_type = sensor_msgs::msg::PointCloud2;
  using msg_pubsub_type = sensor_msgs::msg::PointCloud2PubSubType;
};

namespace {

// Translates the FastDDS-free PointFieldDataType enum (defined in
// PointCloudFieldsLayout.h) to the sensor_msgs::msg::PointField__* numeric
// constants the IDL-generated PointField struct expects on the wire.
std::uint8_t ToPointFieldDatatype(PointFieldDataType datatype) noexcept {
  switch (datatype) {
    case PointFieldDataType::Int8:    return sensor_msgs::msg::PointField__INT8;
    case PointFieldDataType::UInt8:   return sensor_msgs::msg::PointField__UINT8;
    case PointFieldDataType::Int16:   return sensor_msgs::msg::PointField__INT16;
    case PointFieldDataType::UInt16:  return sensor_msgs::msg::PointField__UINT16;
    case PointFieldDataType::Int32:   return sensor_msgs::msg::PointField__INT32;
    case PointFieldDataType::UInt32:  return sensor_msgs::msg::PointField__UINT32;
    case PointFieldDataType::Float32: return sensor_msgs::msg::PointField__FLOAT32;
    case PointFieldDataType::Float64: return sensor_msgs::msg::PointField__FLOAT64;
  }
  return sensor_msgs::msg::PointField__FLOAT32;
}

std::vector<sensor_msgs::msg::PointField> BuildPointFields(
    const PointFieldDescriptor *descriptors, std::size_t count) {
  std::vector<sensor_msgs::msg::PointField> fields;
  fields.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    const auto &descriptor = descriptors[i];
    sensor_msgs::msg::PointField field;
    field.name(std::string{descriptor.name});
    field.offset(descriptor.offset);
    field.datatype(ToPointFieldDatatype(descriptor.datatype));
    field.count(descriptor.count);
    fields.push_back(std::move(field));
  }
  return fields;
}

}  // namespace

CarlaPointCloudPublisher::CarlaPointCloudPublisher(
    std::string base_topic_name, std::string frame_id)
  : BasePublisher(std::move(base_topic_name), std::move(frame_id)),
    _impl(std::make_shared<PublisherImpl<CarlaPointCloudMsgTraits>>()) {
  if (!_impl->Init(GetBaseTopicName() + "/point_cloud")) {
    log_error("CarlaPointCloudPublisher: failed to initialise writer for", GetBaseTopicName());
  }
}

CarlaPointCloudPublisher::~CarlaPointCloudPublisher() = default;

bool CarlaPointCloudPublisher::Publish() {
  return _impl->Publish();
}

bool CarlaPointCloudPublisher::WritePointCloud(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    std::uint32_t height,
    std::uint32_t width,
    const std::uint8_t *data) {
  return WritePointCloud(
      seconds, nanoseconds, height, width,
      ComputePointCloud(height, width, data));
}

bool CarlaPointCloudPublisher::WritePointCloud(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    std::uint32_t height,
    std::uint32_t width,
    std::vector<std::uint8_t> data) {
  auto *message = _impl->GetMessage();
  message->header().stamp().sec(seconds);
  message->header().stamp().nanosec(nanoseconds);
  message->header().frame_id(GetFrameId());

  const std::size_t point_size = GetPointSize();

  message->width(width);
  message->height(height);
  message->is_bigendian(false);
  message->fields(BuildPointFields(GetFieldDescriptors(), GetFieldDescriptorCount()));
  message->point_step(static_cast<std::uint32_t>(point_size));
  message->row_step(static_cast<std::uint32_t>(width * point_size));
  // is_dense=false matches the upstream convention: lidar / radar scans contain
  // invalid points (no return / max-range hits) and subscribers must not
  // assume tightly packed valid data.
  message->is_dense(false);
  message->data(std::move(data));

  return true;
}

}  // namespace ros2
}  // namespace carla
