// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace carla {
namespace ros2 {

/// Type-erased abstract interface for a publisher middleware.
/// Concrete implementations handle all vendor-specific entity creation,
/// type registration, and data writing.
///
/// Contract: when an implementation establishes its process-wide transport
/// context (a DDS DomainParticipant, a Zenoh session, ...) it must honor
/// carla::ros2::MiddlewareConfig::GetDomainId(), mapping kUnsetDomainId to its
/// own native default. This keeps the ROS 2 domain id configurable and
/// middleware-agnostic (see MiddlewareConfig.h).
class IPublisherMiddleware {
 public:
  virtual ~IPublisherMiddleware() = default;

  /// Initialize the underlying middleware entities.
  /// @param topic_name  Full topic name including "rt/" prefix.
  /// @return true on success.
  virtual bool Init(const std::string& topic_name) = 0;

  /// Serialize and write a message to the network.
  /// @param message_data  Pointer to the message object (type-erased, cast internally).
  /// @return true if the write succeeded.
  virtual bool Publish(void* message_data) = 0;

  /// @return true if at least one subscriber is matched.
  virtual bool IsAlive() const = 0;

  /// @return The topic name this publisher is bound to.
  virtual std::string GetTopicName() const = 0;
};

} // namespace ros2
} // namespace carla
