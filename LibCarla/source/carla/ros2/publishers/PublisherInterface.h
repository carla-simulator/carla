// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "builtin_interfaces/msg/Time.h"
#include "carla/BufferView.h"
#include "carla/ros2/ROS2NameRecord.h"
#include "carla/ros2/types/Transform.h"
#include "carla/sensor/data/SerializerVectorAllocator.h"

namespace carla {
namespace ros2 {

/**
 * @brief Generic publisher interface.
 *
 * This interface is used to hide the implementation part of publishers Publish() function.
 * The Publisher inherits this and usually forwards this to the respective publisher impl's it's providing.
 */
class PublisherInterface {
public:
  PublisherInterface() = default;
  virtual ~PublisherInterface() = default;
  /**
   * Copy operation not allowed due to active publisher
   */
  PublisherInterface(const PublisherInterface&) = delete;
  /**
   * Assignment operation not allowed due to active publisher
   */
  PublisherInterface& operator=(const PublisherInterface&) = delete;
  /**
   * Move constructor not allowed due to active publisher.
   */
  PublisherInterface(PublisherInterface&&) = delete;
  /**
   * Move assignment operation not allowed due to active publisher.
   */
  PublisherInterface& operator=(PublisherInterface&&) = delete;

  /**
   * Publish the message
   */
  virtual bool Publish() = 0;

  /**
   * Should return \c true in case there are subscribers connected to the publisher.
   */
  virtual bool SubscribersConnected() const = 0;
};
}  // namespace ros2
}  // namespace carla
