// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/ROS2NameRecord.h"
#include "carla/ros2/ROS2QoS.h"
#include "carla/ros2/publishers/PublisherInterface.h"

namespace carla {
namespace ros2 {

template <typename MESSAGE_TYPE, typename MESSAGE_PUB_TYPE>
class DdsPublisherImpl;

/**
  A Publisher base class for general publisher using default publishing qos.
  Use this class for publisher that need a transform conversion for the TF tree in addition.
 */
class PublisherBase : public PublisherInterface, public ROS2NameRecord {
public:
  PublisherBase(std::shared_ptr<carla::ros2::types::ActorNameDefinition> actor_name_definition)
    : ROS2NameRecord(actor_name_definition) {}
  virtual ~PublisherBase() = default;

  /**
   * Initialze the publisher
   */
  virtual bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) = 0;

  /*
   * @brief Default get_topic_qos() for publishers
   *
   * Be aware: The default selection for publishers is NOT as done default in ROS2 (which aims compatibility to ROS1)!
   * Per default, we want to achieve the most compatible combination within ROS2 world in the sense,
   * that receiption is possible for all possible subscriber configurations.
   * https://docs.ros.org/en/humble/Concepts/Intermediate/About-Quality-of-Service-Settings.html#qos-compatibilities
   *
   * Reliability::RELIABLE
   * Durability::TRANSIENT_LOCAL
   * History::KEEP_LAST, depth: 10u
   */
  ROS2QoS get_topic_qos() const {
    return DEFAULT_PUBLISHER_QOS;
  }

  /*
   * @brief enable actor ROS publication
   */
  void enable_for_ros() {
    _actor_name_definition->enabled_for_ros = true;
  }

  /*
   * @brief disable actor ROS publication
   */
  void disable_for_ros() {
    _actor_name_definition->enabled_for_ros = false;
  }

  /*
   * @brief is the publisher actually enabled for ROS publication
   */
  bool is_enabled_for_ros() const {
    return _actor_name_definition->enabled_for_ros;
  }
};
}  // namespace ros2
}  // namespace carla
