// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace carla {
namespace ros2 {

/// Type-erased abstract interface for a DDS subscriber middleware.
/// Concrete implementations write received messages directly into the caller-provided
/// storage (message_ptr / new_message_flag) to avoid an extra copy.
class IDDSSubscriberMiddleware {
 public:
  virtual ~IDDSSubscriberMiddleware() = default;

  /// Initialize DDS entities (participant, subscriber, topic, reader).
  /// The middleware writes incoming messages to *message_ptr and sets *new_message_flag = true.
  /// @param topic_name       Full DDS topic name.
  /// @param message_ptr      Pointer to the message storage owned by SubscriberImpl<S>.
  /// @param new_message_flag Pointer to the new-message flag owned by SubscriberImpl<S>.
  /// @return true on success.
  virtual bool Init(
      const std::string& topic_name,
      void* message_ptr,
      bool* new_message_flag) = 0;

  /// @return true if at least one publisher is matched.
  virtual bool IsAlive() const = 0;

  /// @return The DDS topic name this subscriber is bound to.
  virtual std::string GetTopicName() const = 0;
};

} // namespace ros2
} // namespace carla
