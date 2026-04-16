// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#ifndef CARLA_ROS2_DDS_TESTING

#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace carla {
namespace ros2 {

/// Refcounted singleton DomainParticipant shared across all FastDDS
/// publishers and subscribers in the process.
///
/// Using one DomainParticipant per publisher (the previous approach) caused a
/// discovery storm on ROS2::Shutdown() when all N participants were destroyed
/// back-to-back, triggering intermittent segfaults in remote rmw_fastrtps_cpp
/// and rmw_cyclonedds_cpp clients.  CycloneDDS already uses a
/// process-lifetime shared participant (CycloneDDSSertype::carla_cdr_get_participant);
/// this class mirrors that model for FastDDS.
///
/// Thread-safety: all methods acquire an internal mutex.
class FastDDSSharedParticipant {
 public:
  /// Increment the refcount and return the shared DomainParticipant.
  /// Creates the participant on the first call.  Returns nullptr on failure.
  static eprosima::fastdds::dds::DomainParticipant* acquire();

  /// Decrement the refcount.  Destroys the participant when count reaches 0.
  /// Must be called exactly once for every successful acquire().
  static void release();

  /// Notify the singleton that one more endpoint is using type_name so that
  /// unregister_type() is deferred until the last user releases it.
  static void retain_type(const std::string& type_name);

  /// Release one use of type_name.  Calls participant->unregister_type() when
  /// the count for that name drops to 0 (and the participant is still alive).
  static void release_type(const std::string& type_name);

 private:
  FastDDSSharedParticipant() = delete;

  static std::mutex                       _mutex;
  static eprosima::fastdds::dds::DomainParticipant* _participant;
  static uint32_t                         _refcount;
  static std::unordered_map<std::string, uint32_t>  _type_refcounts;
};

} // namespace ros2
} // namespace carla

#endif // !CARLA_ROS2_DDS_TESTING
