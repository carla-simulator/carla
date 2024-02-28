// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include "carla/ros2/ROS2QoS.h"

namespace carla {
namespace ros2 {

inline eprosima::fastdds::dds::TopicQos TopicQos(ROS2QoS const &qos) {
  eprosima::fastdds::dds::TopicQos topic_qos = eprosima::fastdds::dds::TOPIC_QOS_DEFAULT;

  if (qos._reliability == ROS2QoS::Reliability::BEST_EFFORT) {
    topic_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
  } else if (qos._reliability == ROS2QoS::Reliability::RELIABLE) {
    topic_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
  }

  if (qos._durability == ROS2QoS::Durability::VOLATILE) {
    topic_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS;
  } else if (qos._durability == ROS2QoS::Durability::TRANSIENT_LOCAL) {
    topic_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
  }

  if (qos._history == ROS2QoS::History::KEEP_LAST) {
    topic_qos.history().kind = eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
    topic_qos.history().depth = qos._history_depth;
  } else if (qos._history == ROS2QoS::History::KEEP_ALL) {
    topic_qos.history().kind = eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
    topic_qos.history().depth = qos._history_depth;
  }
  return topic_qos;
}

}  // namespace ros2
}  // namespace carla