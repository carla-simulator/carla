// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef CARLA_ROS2_MIDDLEWARE_TESTING
#ifdef CARLA_ROS2_MIDDLEWARE_ZENOH

#include "carla/ros2/middleware/zenoh/ZenohSharedSession.h"
#include "carla/ros2/middleware/zenoh/ZenohWireFormat.h"
#include "carla/Logging.h"

namespace carla {
namespace ros2 {

namespace {

std::string compute_zid_string(const z_loaned_session_t* session) {
  z_id_t zid = z_info_zid(session);
  z_owned_string_t s;
  z_id_to_string(&zid, &s);
  const z_loaned_string_t* ls = z_loan(s);
  std::string result(z_string_data(ls), z_string_len(ls));
  z_drop(z_move(s));
  return result;
}

// __FILE__ resolves to this .cpp's absolute path at compile time; the
// shipped default config sits next to it under config/. Works in editor
// mode where the source tree is still in place.
std::string default_config_path() {
  std::string src = __FILE__;
  return src.substr(0, src.find_last_of('/')) +
         "/config/zenoh_session_config.json5";
}

// rmw_zenoh-compatible: ZENOH_CONFIG_OVERRIDE='k1/k2=v1;k3=v2' patches
// individual keys on top of the loaded config.
void apply_config_overrides(z_loaned_config_t* cfg) {
  const char* env = std::getenv("ZENOH_CONFIG_OVERRIDE");
  if (env == nullptr || env[0] == '\0') return;

  std::string s(env);
  size_t pos = 0;
  while (pos < s.size()) {
    size_t semi = s.find(';', pos);
    if (semi == std::string::npos) semi = s.size();
    std::string pair = s.substr(pos, semi - pos);
    size_t eq = pair.find('=');
    if (eq != std::string::npos) {
      std::string key = pair.substr(0, eq);
      std::string val = pair.substr(eq + 1);
      if (zc_config_insert_json5(cfg, key.c_str(), val.c_str()) != Z_OK) {
        log_warning("ZENOH_CONFIG_OVERRIDE: failed to apply '", pair, "'");
      }
    }
    pos = semi + 1;
  }
}

}  // namespace

const z_loaned_session_t* zenoh_get_shared_session() {
  static z_owned_session_t session = [] {
    z_owned_config_t cfg;
    // rmw_zenoh-compatible: ZENOH_SESSION_CONFIG_URI swaps the whole file;
    // otherwise use the shipped default sitting next to this .cpp.
    const char* uri = std::getenv("ZENOH_SESSION_CONFIG_URI");
    const std::string path = (uri != nullptr && uri[0] != '\0')
        ? std::string(uri) : default_config_path();
    if (zc_config_from_file(&cfg, path.c_str()) != Z_OK) {
      log_error("ZenohSharedSession: failed to read config at '", path,
                "'; falling back to zenoh defaults");
      z_config_default(&cfg);
    }
    apply_config_overrides(z_loan_mut(cfg));

    z_owned_session_t s;
    if (z_open(&s, z_move(cfg), nullptr) != Z_OK) {
      log_error("ZenohSharedSession: z_open failed");
    }
    return s;
  }();

  // The function-local static keeps the carla_bridge node liveliness token
  // alive for the process lifetime so rmw_zenoh peers continue to see the
  // node in their graph cache for as long as the session is up.
  static z_owned_liveliness_token_t node_token = [] {
    const z_loaned_session_t* loaned = z_session_loan(&session);
    const std::string ke_str = zenoh_make_node_liveliness_keyexpr(
        zenoh_ros_domain_id(), compute_zid_string(loaned));
    z_view_keyexpr_t ke;
    z_view_keyexpr_from_str(&ke, ke_str.c_str());
    z_owned_liveliness_token_t tok;
    if (z_liveliness_declare_token(loaned, &tok, z_loan(ke), nullptr) != Z_OK) {
      log_error("ZenohSharedSession: z_liveliness_declare_token failed");
    }
    return tok;
  }();
  (void)node_token;

  return z_session_loan(&session);
}

const std::string& zenoh_session_zid() {
  static const std::string cached =
      compute_zid_string(zenoh_get_shared_session());
  return cached;
}

}  // namespace ros2
}  // namespace carla

#endif // CARLA_ROS2_MIDDLEWARE_ZENOH
#endif // !CARLA_ROS2_MIDDLEWARE_TESTING
