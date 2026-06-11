// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace ros2 {

/// Durability of a publisher endpoint, mirroring the DDS durability QoS.
enum class DurabilityKind : uint8_t {
  /// Late-joining subscribers only receive samples written after they match.
  Volatile,
  /// The writer keeps a sample cache so late-joining subscribers receive the
  /// last history_depth samples (the ROS 2 "latched topic" behavior).
  TransientLocal
};

/// QoS settings applied to a publisher middleware at Init time.
/// The defaults reproduce the exact behavior every publisher had before QoS
/// support was added: volatile durability with a keep-last history of 1.
struct PublisherQos {
  DurabilityKind durability{DurabilityKind::Volatile};
  uint32_t history_depth{1u};

  /// History depth to hand to the middleware: a keep-last history needs a
  /// positive depth, so the invalid value 0 is clamped to 1.
  uint32_t effective_history_depth() const {
    return history_depth == 0u ? 1u : history_depth;
  }
};

} // namespace ros2
} // namespace carla
