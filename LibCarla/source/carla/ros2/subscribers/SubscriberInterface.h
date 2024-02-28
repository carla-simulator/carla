// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace ros2 {

template <typename MESSAGE_TYPE>
class SubscriberInterface {
public:
  /**
   * Default constructor.
   */
  SubscriberInterface() = default;
  /**
   * Copy operation not allowed due to active subscriptions
   */
  SubscriberInterface(const SubscriberInterface&) = delete;
  /**
   * Assignment operation not allowed due to active subscriptions
   */
  SubscriberInterface& operator=(const SubscriberInterface&) = delete;
  /**
   * Move operation not allowed due to active subscriptions
   */
  SubscriberInterface(SubscriberInterface&&) = delete;
  /**
   * Move operation not allowed due to active subscriptions
   */
  SubscriberInterface& operator=(SubscriberInterface&&) = delete;

  /**
   * Default destructor.
   */
  virtual ~SubscriberInterface() = default;

  /**
   * Check if the connection is still alive
   */
  virtual bool IsAlive() const = 0;

  /**
   * Check if there is a new message available
   */
  virtual bool HasNewMessage() const = 0;

  /**
   * Get the current message and reset the HasNewMessage() flag
   */
  virtual const MESSAGE_TYPE& GetMessage() = 0;
};
}  // namespace ros2
}  // namespace carla
