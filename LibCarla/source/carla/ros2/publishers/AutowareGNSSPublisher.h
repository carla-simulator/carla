// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/BasePublisher.h"

#include <cstdint>
#include <memory>

namespace carla {
namespace ros2 {

// Port of tier4's AutowareGNSSPublisher: a dual-topic pose publisher fed by
// the Autoware GNSS sensor. Publishes the sensor's *world* transform (plus an
// optional MGRS map offset) instead of geodetic coordinates:
//   <base_topic>/pose                 geometry_msgs/Pose (bare pose, no header)
//   <base_topic>/pose_with_covariance geometry_msgs/PoseWithCovarianceStamped
// frame_id (used in the stamped message header) is "map" per tier4's design.
// QoS RELIABLE / VOLATILE / KEEP_LAST depth 1 per tier4.
// Dual-topic structure mirrors CarlaCameraPublisher (image_raw + camera_info).
class AutowareGNSSPublisher : public BasePublisher {
public:
  AutowareGNSSPublisher(std::string base_topic_name, std::string frame_id);
  ~AutowareGNSSPublisher() override;

  AutowareGNSSPublisher(const AutowareGNSSPublisher &) = delete;
  AutowareGNSSPublisher &operator=(const AutowareGNSSPublisher &) = delete;

  /// Fill both pose messages from the sensor's CARLA world transform
  /// (translation in meters, rotation pitch/yaw/roll in degrees) plus the
  /// MGRS map-origin offset in meters. Pass {0,0,0} offsets when no
  /// georeference is configured.
  bool Write(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      float tx, float ty, float tz,
      float pitch_deg, float yaw_deg, float roll_deg,
      double mgrs_offset_x, double mgrs_offset_y, double mgrs_offset_z);

  /// Publishes both topics.
  bool Publish() override;

private:
  class Impl;
  std::shared_ptr<Impl> _impl;
};

}  // namespace ros2
}  // namespace carla
