// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// Pure keyexpr/wire-format helpers with no zenoh-c dependency; also compiled
// under CARLA_ROS2_MIDDLEWARE_TESTING so unit tests cover them.
#if defined(CARLA_ROS2_MIDDLEWARE_ZENOH) || defined(CARLA_ROS2_MIDDLEWARE_TESTING)

#include "carla/ros2/middleware/MiddlewareConfig.h"

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

namespace carla {
namespace ros2 {

// rmw_zenoh wire-format constants. Values mirror the ones used by
// zenoh_carla_bridge and by rmw_zenoh peers so all sides speak the same
// dialect. See https://github.com/ros2/rmw_zenoh/blob/rolling/docs/design.md.
constexpr const char* kZenohNodeId         = "0";
constexpr const char* kZenohNodeName       = "carla_bridge";
constexpr const char* kZenohEntityKindPub  = "MP";
constexpr const char* kZenohEntityKindSub  = "MS";
constexpr const char* kZenohEntityKindNode = "NN";
constexpr const char* kZenohDefaultQos     = "::,1:,:,:,,";

/// Monotonic per-process counter used to assign a unique entity id (eid) to
/// each publisher and subscriber declared on the shared Session.
inline uint64_t zenoh_next_entity_id() {
  static std::atomic<uint64_t> counter{0};
  return counter.fetch_add(1, std::memory_order_relaxed);
}

/// Effective ROS 2 domain id as a string, resolved by the middleware
/// abstraction layer (--ros-domain-id, else ROS_DOMAIN_ID, else the default
/// domain 0). Populates the leading segment of every keyexpr so we only match
/// rmw_zenoh peers on the same domain, mirroring the domain id the
/// FastDDS/CycloneDDS middlewares pass to their DomainParticipant.
inline std::string zenoh_ros_domain_id() {
  return std::to_string(MiddlewareConfig::GetEffectiveDomainId());
}

/// Strip the "rt/" prefix that ROS2-to-DDS mapping adds to message topics.
/// rmw_zenoh keyexprs and liveliness tokens use the bare topic name.
inline std::string zenoh_strip_rt_prefix(const std::string& topic) {
  return topic.compare(0, 3, "rt/") == 0 ? topic.substr(3) : topic;
}

/// Replace '/' with '%' so the topic appears as a single segment inside the
/// slash-separated liveliness keyexpr (e.g. "vehicle/imu" -> "%vehicle%imu").
inline std::string zenoh_mangle_topic(const std::string& topic) {
  std::string out;
  out.reserve(topic.size() + 1u);
  out.push_back('%');
  for (char c : topic) out.push_back(c == '/' ? '%' : c);
  return out;
}

/// Keyexpr used by declare_publisher / declare_subscriber. Format:
/// "<domain>/<topic>/<type>/<hash>".
inline std::string zenoh_make_topic_keyexpr(
    const std::string&        domain,
    const std::string&        topic_no_rt,
    const char*               type_name,
    [[maybe_unused]] const char* type_hash)
{
  return domain + "/" + topic_no_rt + "/" + type_name + "/*";
}

/// Liveliness keyexpr that registers carla_bridge as a node in rmw_zenoh's
/// graph cache. Format: "@ros2_lv/<domain>/<zid>/0/0/NN/%/%/carla_bridge".
inline std::string zenoh_make_node_liveliness_keyexpr(
    const std::string& domain,
    const std::string& zid)
{
  return std::string("@ros2_lv/") + domain + "/" + zid + "/" + kZenohNodeId +
         "/" + kZenohNodeId + "/" + kZenohEntityKindNode + "/%/%/" +
         kZenohNodeName;
}

/// Liveliness keyexpr that registers one pub/sub endpoint of carla_bridge.
/// Format:
/// "@ros2_lv/<domain>/<zid>/0/<eid>/<MP|MS>/%/%/carla_bridge/<mangled>/<type>/<hash>/<qos>".
inline std::string zenoh_make_topic_liveliness_keyexpr(
    const std::string& domain,
    const std::string& zid,
    uint64_t           eid,
    const char*        entity_kind,
    const std::string& topic_no_rt,
    const char*        type_name,
    const char*        type_hash,
    const char*        qos)
{
  return std::string("@ros2_lv/") + domain + "/" + zid + "/" + kZenohNodeId +
         "/" + std::to_string(eid) + "/" + entity_kind + "/%/%/" +
         kZenohNodeName + "/" + zenoh_mangle_topic(topic_no_rt) + "/" +
         type_name + "/" + type_hash + "/" + qos;
}

/// 33-byte rmw_zenoh attachment payload sent with every put(). Layout:
/// seq:i64 LE | ts:i64 LE | gid_len:u8(=16) | gid:[u8;16]. Constants are
/// copied verbatim from zenoh_carla_bridge's utils.rs::generate_attachment.
inline std::vector<uint8_t> zenoh_attachment() {
  return {
      1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      16,
      0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB,
      0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB,
  };
}

} // namespace ros2
} // namespace carla

#endif // CARLA_ROS2_MIDDLEWARE_ZENOH || CARLA_ROS2_MIDDLEWARE_TESTING
