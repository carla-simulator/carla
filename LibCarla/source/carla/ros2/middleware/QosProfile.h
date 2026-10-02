// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace ros2 {

/// Middleware-neutral QoS profile passed to IPublisherMiddleware::Init /
/// ISubscriberMiddleware::Init when a topic needs QoS different from the
/// vendor defaults (e.g. the Autoware command subscribers require
/// RELIABLE + TRANSIENT_LOCAL to latch the last command).
///
/// This replaces tier4's TopicConfig (data_types.h) in the
/// middleware-decoupled architecture: the domain id is NOT part of the
/// profile (it lives in MiddlewareConfig.h) and the topic suffix is the
/// caller's concern (publishers append suffixes to their base topic name).
///
/// The defaults (RELIABLE / VOLATILE / KEEP_LAST depth 1) mirror the FastDDS
/// DataWriter defaults so passing QosProfile{} to a publisher is equivalent
/// to the QoS-less Init overload.
struct QosProfile {
  enum class Reliability : std::uint8_t { Reliable, BestEffort };
  enum class Durability  : std::uint8_t { Volatile, TransientLocal };
  enum class History     : std::uint8_t { KeepLast, KeepAll };

  Reliability  reliability{Reliability::Reliable};
  Durability   durability{Durability::Volatile};
  History      history{History::KeepLast};
  std::int32_t history_depth{1};

  /// RELIABLE / TRANSIENT_LOCAL / KEEP_LAST depth 1 — the profile tier4 uses
  /// for the Autoware command subscribers (latched commands survive a late
  /// subscriber join).
  static QosProfile ReliableTransientLocal() {
    QosProfile p;
    p.reliability = Reliability::Reliable;
    p.durability = Durability::TransientLocal;
    p.history = History::KeepLast;
    p.history_depth = 1;
    return p;
  }

  /// RELIABLE / VOLATILE / KEEP_LAST depth 1 — the profile tier4 uses for the
  /// Autoware report publishers.
  static QosProfile ReliableVolatile() {
    QosProfile p;
    p.reliability = Reliability::Reliable;
    p.durability = Durability::Volatile;
    p.history = History::KeepLast;
    p.history_depth = 1;
    return p;
  }
};

} // namespace ros2
} // namespace carla
