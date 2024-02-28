// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <stdint.h>

namespace carla {
namespace ros2 {

/*
 * Struct providing the most prominent ROS2 ROS2QoS parameters
 * Default values are selected to be the default used by the ROS2.
 *
 * Reliability::RELIABLE
 * Durability::VOLATILE
 * History::KEEP_LAST, depth: 10u
 */
struct ROS2QoS {
  ROS2QoS() = default;
  ~ROS2QoS() = default;
  ROS2QoS(const ROS2QoS&) = default;
  ROS2QoS& operator=(const ROS2QoS&) = default;
  ROS2QoS(ROS2QoS&&) = default;
  ROS2QoS& operator=(ROS2QoS&&) = default;

  enum class Reliability { SYSTEM_DEFAULT, BEST_EFFORT, RELIABLE } _reliability;

  enum class Durability { SYSTEM_DEFAULT, TRANSIENT_LOCAL, VOLATILE } _durability;

  enum class History { SYSTEM_DEFAULT, KEEP_LAST, KEEP_ALL } _history;

  int32_t _history_depth;
};

static constexpr ROS2QoS DEFAULT_ROS2_QOS{ROS2QoS::Reliability::RELIABLE, ROS2QoS::Durability::VOLATILE,
                                          ROS2QoS::History::KEEP_LAST, 10};

static constexpr ROS2QoS DEFAULT_SENSOR_DATA_QOS{ROS2QoS::Reliability::BEST_EFFORT, ROS2QoS::Durability::VOLATILE,
                                                 ROS2QoS::History::KEEP_LAST, 5};

static constexpr ROS2QoS DEFAULT_SUBSCRIBER_QOS{ROS2QoS::Reliability::BEST_EFFORT, ROS2QoS::Durability::VOLATILE,
                                                ROS2QoS::History::KEEP_LAST, 10};

static constexpr ROS2QoS DEFAULT_PUBLISHER_QOS{ROS2QoS::Reliability::RELIABLE, ROS2QoS::Durability::TRANSIENT_LOCAL,
                                               ROS2QoS::History::KEEP_LAST, 10};

}  // namespace ros2
}  // namespace carla
