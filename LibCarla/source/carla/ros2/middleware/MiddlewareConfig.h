// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdlib>

namespace carla {
namespace ros2 {

/// Sentinel domain id meaning "no domain id was configured on the command line";
/// each middleware falls back to its own native default.
constexpr int kUnsetDomainId = -1;

/// Lowest valid ROS 2 domain id.
constexpr int kMinDomainId = 0;

/// Highest valid ROS 2 domain id (RTPS maximum).
constexpr int kMaxDomainId = 232;

/// Domain id used when neither the command line nor the environment provides one.
constexpr int kDefaultDomainId = 0;

/// Name of the standard ROS 2 environment variable that selects the domain id.
constexpr char kRosDomainIdEnvVar[] = "ROS_DOMAIN_ID";

/// @return true if @a id is a valid ROS 2 domain id (0..232).
/// The unset sentinel is intentionally NOT valid: callers test for it explicitly.
inline bool IsValidDomainId(int id) {
  return id >= kMinDomainId && id <= kMaxDomainId;
}

/// Where the effective domain id came from, in priority order.
enum class DomainIdSource {
  CommandLine,   ///< From the --ros-domain-id command line option.
  Environment,   ///< From the ROS_DOMAIN_ID environment variable.
  Default        ///< Neither was set; the default domain (0) is used.
};

/// The resolved domain id together with the source it came from.
struct ResolvedDomainId {
  int id;
  DomainIdSource source;
};

/// Parse a ROS_DOMAIN_ID-style string into a valid domain id.
/// @return true and writes @a out only when @a text is a base-10 integer that is
/// a valid domain id (0..232). nullptr, empty, non-numeric, trailing-garbage and
/// out-of-range inputs return false and leave @a out untouched.
inline bool TryParseDomainId(const char* text, int& out) {
  if (text == nullptr) {
    return false;
  }
  char* end = nullptr;
  const long value = std::strtol(text, &end, 10);
  if (end == text) {
    return false;  // No digits were consumed.
  }
  while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r') {
    ++end;
  }
  if (*end != '\0') {
    return false;  // Trailing non-numeric characters.
  }
  if (value < kMinDomainId || value > kMaxDomainId) {
    return false;
  }
  out = static_cast<int>(value);
  return true;
}

/// Resolve the effective domain id from a command line value and an environment
/// value, in priority order:
///   1. @a cli_value, when it is a valid domain id (0..232).
///   2. @a env_value (e.g. ROS_DOMAIN_ID), when it parses to a valid domain id.
///   3. kDefaultDomainId (0).
/// This function is pure (no environment access), so the precedence rules can be
/// unit-tested by passing the environment value as a string.
inline ResolvedDomainId ResolveDomainId(int cli_value, const char* env_value) {
  if (IsValidDomainId(cli_value)) {
    return {cli_value, DomainIdSource::CommandLine};
  }
  int parsed = 0;
  if (TryParseDomainId(env_value, parsed)) {
    return {parsed, DomainIdSource::Environment};
  }
  return {kDefaultDomainId, DomainIdSource::Default};
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

  /// Resolve the effective domain id and its source from the configured value,
  /// the ROS_DOMAIN_ID environment variable, and the default. See ResolveDomainId
  /// for the priority order.
  static ResolvedDomainId ResolveEffective() {
    return ResolveDomainId(GetDomainId(), std::getenv(kRosDomainIdEnvVar));
  }

  /// @return The effective domain id every middleware should use, after applying
  /// the command-line value, the ROS_DOMAIN_ID environment variable and the
  /// default in that order.
  static int GetEffectiveDomainId() {
    return ResolveEffective().id;
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
