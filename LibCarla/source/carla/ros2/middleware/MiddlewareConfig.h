// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace ros2 {

/// Sentinel domain id meaning "no domain id was configured on the command line";
/// each middleware falls back to its own native default.
constexpr int kUnsetDomainId = -1;

/// Lowest valid ROS 2 domain id.
constexpr int kMinDomainId = 0;

/// Highest valid ROS 2 domain id (RTPS maximum).
constexpr int kMaxDomainId = 232;

/// @return true if @a id is a valid ROS 2 domain id (0..232).
/// The unset sentinel is intentionally NOT valid: callers test for it explicitly.
inline bool IsValidDomainId(int id) {
  return id >= kMinDomainId && id <= kMaxDomainId;
}

/// Process-wide runtime configuration shared by every ROS 2 middleware
/// implementation (FastDDS, CycloneDDS, and future ones such as Zenoh).
///
/// This belongs to the middleware abstraction layer, not to any single vendor:
/// each concrete middleware reads this configuration when it establishes its
/// transport context (a DDS DomainParticipant, a Zenoh session, ...) and
/// translates the abstract values into its own mechanism, mapping
/// kUnsetDomainId to its native default.
///
/// Set once at startup (via ROS2::Enable) before any publisher or subscriber is
/// created. Reads after that point are stable. Because it is set before any
/// entity exists, no synchronisation is required.
class MiddlewareConfig {
 public:
  /// Configure the ROS 2 domain id for all subsequent transport contexts.
  /// Pass kUnsetDomainId to keep each middleware's native default.
  static void SetDomainId(int id) {
    Storage().domain_id = id;
  }

  /// @return The configured domain id, or kUnsetDomainId if none was set.
  static int GetDomainId() {
    return Storage().domain_id;
  }

 private:
  struct Data {
    int domain_id = kUnsetDomainId;
  };

  /// Function-local static: a single instance shared across all translation units.
  static Data& Storage() {
    static Data data;
    return data;
  }
};

} // namespace ros2
} // namespace carla
