// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/types/CdrTopicInfo.h"

#include <cstdint>
#include <cstring>
#include <vector>

namespace carla {
namespace ros2 {

/// Builds the REP-2016 endpoint USER_DATA payload encoding a REP-2011 type
/// hash.
///
/// Per OMG DDSI-RTPS v2.5 §9.6.2.2.2, PID_USER_DATA (0x002c) carries the
/// DDS UserDataQosPolicy octet sequence.  ROS 2 REP-2016 defines the payload
/// as a set of ASCII key=value pairs separated by semicolons, e.g.:
///
///   typehash=RIHS01_<64 lowercase hex>;
///
/// rmw_cyclonedds_cpp and rmw_fastrtps_cpp on Iron / Jazzy parse this string
/// from PID_USER_DATA during SEDP endpoint discovery to perform REP-2011
/// type-hash-based matching.  When it is absent (null), Jazzy rmws emit:
///   [WARN] Failed to parse type hash ... from USER_DATA '(null)'.
///
/// This helper is vendor-agnostic; the resulting byte vector is fed to:
///   FastDDS : DataWriterQos::user_data().data_vec()
///             DataReaderQos::user_data().data_vec()
///   CycloneDDS: dds_qset_userdata(qos, data, size)
///
/// If type_hash is nullptr (used for types whose hash is unknown), an empty
/// vector is returned and user_data should NOT be set.
inline std::vector<uint8_t> build_user_data(const char* type_hash) {
  if (type_hash == nullptr) {
    return {};
  }
  // Payload: "typehash=<hash>;"  (no null terminator — DDS carries length)
  static const char prefix[] = "typehash=";
  static const char suffix[] = ";";
  const size_t hash_len  = std::strlen(type_hash);
  const size_t total_len = sizeof(prefix) - 1u + hash_len + sizeof(suffix) - 1u;

  std::vector<uint8_t> payload;
  payload.reserve(total_len);
  for (const char* p = prefix; *p; ++p) payload.push_back(static_cast<uint8_t>(*p));
  for (size_t i = 0u; i < hash_len; ++i) payload.push_back(static_cast<uint8_t>(type_hash[i]));
  for (const char* p = suffix; *p; ++p) payload.push_back(static_cast<uint8_t>(*p));
  return payload;
}

/// Convenience: build user_data from the CdrTopicInfo<T>::type_hash() of T.
/// Returns an empty vector (and the caller must skip setting user_data) when
/// CdrTopicInfo<T>::type_hash() returns nullptr.
template<typename T>
inline std::vector<uint8_t> build_user_data_for() {
  return build_user_data(CdrTopicInfo<T>::type_hash());
}

} // namespace ros2
} // namespace carla
