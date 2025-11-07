// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/PublisherBaseSensor.h"

#include "carla/ros2/publishers/TransformPublisher.h"
#include "carla/ros2/types/CoordinateSystemTransform.h"
#include "carla/ros2/types/Timestamp.h"
#include "carla/ros2/types/Transform.h"
#include "carla/sensor/s11n/SensorHeaderSerializer.h"

namespace carla {
namespace ros2 {

/**
  A Publisher base class that is extended to store an internal Transform.
  Use this class for publisher that need a transform conversion for the TF tree in addition.
 */
class PublisherBaseTransform : public PublisherBaseSensor {
public:
  using CoordinateSystemTransform = carla::ros2::types::CoordinateSystemTransform;

  PublisherBaseTransform(std::shared_ptr<carla::ros2::types::ActorNameDefinition> actor_name_definition,
                         std::shared_ptr<TransformPublisher> transform_publisher)
    : PublisherBaseSensor(actor_name_definition), _transform_publisher(transform_publisher) {}
  virtual ~PublisherBaseTransform() = default;

  /**
   * Update the internal transform state with the new transform.
   */
  void UpdateTransform(std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header) {
    UpdateTransform(ros2::types::Timestamp(sensor_header->timestamp),
                    ros2::types::Transform(sensor_header->sensor_relative_transform,
                                           sensor_header->sensor_relative_transform_quaternion));
  }

  /**
   * Update the internal transform state with the new transform.
   */
  void UpdateTransform(ros2::types::Timestamp const &ros_timestamp, ros2::types::Transform const &ros_transform) {
    _timestamp = ros_timestamp;
    _transform = ros_transform;
    _transform_publisher->AddTransform(_timestamp.time(), frame_id(), parent_frame_id(), _transform.transform());
  }

  /**
   * The resulting ROS geometry_msgs::msg::Accel
   */
  geometry_msgs::msg::Transform transform() const {
    return _transform.transform();
  }

  /**
   * The input carla location
   */
  carla::geom::Location const &GetLocation() const {
    return _transform.GetLocation();
  }

  /**
   * The input carla quaternion
   */
  carla::geom::Quaternion const &GetQuaternion() const {
    return _transform.GetQuaternion();
  }

protected:
  carla::ros2::types::Timestamp _timestamp;
  carla::ros2::types::Transform _transform;
  std::shared_ptr<TransformPublisher> _transform_publisher;
};
}  // namespace ros2
}  // namespace carla
