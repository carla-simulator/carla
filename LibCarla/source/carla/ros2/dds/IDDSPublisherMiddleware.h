// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace carla {
namespace ros2 {

/// Type-erased abstract interface for a DDS publisher middleware.
/// Concrete implementations handle all vendor-specific DDS entity creation,
/// type registration, and data writing.
class IDDSPublisherMiddleware {
 public:
  virtual ~IDDSPublisherMiddleware() = default;

  /// Initialize DDS entities (participant, publisher, topic, writer).
  /// @param topic_name  Full DDS topic name including "rt/" prefix.
  /// @return true on success.
  virtual bool Init(const std::string& topic_name) = 0;

  /// Serialize and write a message to the DDS network.
  /// @param message_data  Pointer to the message object (type-erased, cast internally).
  /// @return true if the write succeeded.
  virtual bool Publish(void* message_data) = 0;

  /// @return true if at least one subscriber is matched.
  virtual bool IsAlive() const = 0;

  /// @return The DDS topic name this publisher is bound to.
  virtual std::string GetTopicName() const = 0;
};

} // namespace ros2
} // namespace carla
