// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include "carla/ros2/ROS2QoS.h"

namespace carla {
namespace ros2 {

template <typename FAST_DDS_QOS_TYPE, FAST_DDS_QOS_TYPE const &fast_dds_default_qos>
FAST_DDS_QOS_TYPE FastDdsQos(ROS2QoS const &qos) {
  FAST_DDS_QOS_TYPE fast_dds_qos = fast_dds_default_qos;

  if (qos._reliability == ROS2QoS::Reliability::BEST_EFFORT) {
    fast_dds_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
  } else if (qos._reliability == ROS2QoS::Reliability::RELIABLE) {
    fast_dds_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
  }

  if (qos._durability == ROS2QoS::Durability::VOLATILE) {
    fast_dds_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS;
  } else if (qos._durability == ROS2QoS::Durability::TRANSIENT_LOCAL) {
    fast_dds_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
  }

  if (qos._history == ROS2QoS::History::KEEP_LAST) {
    fast_dds_qos.history().kind = eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
    fast_dds_qos.history().depth = qos._history_depth;
  } else if (qos._history == ROS2QoS::History::KEEP_ALL) {
    fast_dds_qos.history().kind = eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
    fast_dds_qos.history().depth = qos._history_depth;
  }
  return fast_dds_qos;
}

inline eprosima::fastdds::dds::TopicQos TopicQos(ROS2QoS const &qos) {
  return FastDdsQos<eprosima::fastdds::dds::TopicQos, eprosima::fastdds::dds::TOPIC_QOS_DEFAULT>(qos);
}

inline eprosima::fastdds::dds::DataWriterQos DataWriterQos(ROS2QoS const &qos) {
  return FastDdsQos<eprosima::fastdds::dds::DataWriterQos, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT>(qos);
}

inline eprosima::fastdds::dds::DataReaderQos DataReaderQos(ROS2QoS const &qos) {
  return FastDdsQos<eprosima::fastdds::dds::DataReaderQos, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT>(qos);
}

inline eprosima::fastdds::dds::PublisherQos PublisherQos(ROS2QoS const &qos) {
  (void)qos;
  eprosima::fastdds::dds::PublisherQos pubqos = eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT;
  return pubqos;
}

inline eprosima::fastdds::dds::SubscriberQos SubscriberQos(ROS2QoS const &qos) {
  (void)qos;
  eprosima::fastdds::dds::SubscriberQos subqos = eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT;
  return subqos;
}

}  // namespace ros2
}  // namespace carla