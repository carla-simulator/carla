// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/ROS2NameRecord.h"
#include "carla/ros2/ROS2QoS.h"
#include "carla/ros2/subscribers/SubscriberInterface.h"

namespace carla {
namespace ros2 {

/**
  A Subscriber base class.
  */
template <typename MESSAGE_TYPE, typename MESSAGE_PUB_TYPE>
class DdsSubscriberImpl;

template <typename MESSAGE_TYPE>
class SubscriberBase : public SubscriberInterface<MESSAGE_TYPE> {
public:
  SubscriberBase(ROS2NameRecord &parent) : _parent(parent) {}
  virtual ~SubscriberBase() = default;

  /**
   * Initialze the subscriber
   */
  virtual bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) = 0;

  /**
   * Process all available messages.
   */
  virtual void ProcessMessages() = 0;

  /*
   * @brief Default get_topic_qos() for subscribers
   *
   * Be aware: The default selection for subscribers is NOT as done default in ROS2 (which aims compatibility to ROS1)!
   * Per default, we want to achieve the most compatible combination within ROS2 world in the sense,
   * that receiption is possible for all possible publisher configurations.
   * https://docs.ros.org/en/humble/Concepts/Intermediate/About-Quality-of-Service-Settings.html#qos-compatibilities
   *
   * Reliability::BEST_EFFORT
   * Durability::VOLATILE
   * History::KEEP_LAST, depth: 10u
   */
  ROS2QoS get_topic_qos() const {
    return DEFAULT_SUBSCRIBER_QOS;
  };

protected:
  ROS2NameRecord &_parent;
};
}  // namespace ros2
}  // namespace carla
