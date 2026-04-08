// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/dds/IDDSSubscriberMiddleware.h"
#include "carla/Logging.h"

namespace carla {
namespace ros2 {

/// CycloneDDS implementation of IDDSSubscriberMiddleware (stub).
///
/// This is a placeholder that compiles and satisfies the factory interface
/// but does not contain a real CycloneDDS implementation. All operations
/// log an error and return failure. The real implementation will replace
/// this file once CycloneDDS types and the C API are available.
///
/// Parameterized on a traits type S that provides:
///   S::msg_type  — middleware-neutral POD message struct
template<typename S>
class CycloneDDSSubscriberMiddleware : public IDDSSubscriberMiddleware {
 public:
  bool Init(
      const std::string& topic_name,
      void* /*message_ptr*/,
      bool* /*new_message_flag*/) override {
    log_error("CycloneDDSSubscriberMiddleware: stub — not yet implemented "
              "(topic '", topic_name, "')");
    return false;
  }

  bool IsAlive() const override {
    return false;
  }

  std::string GetTopicName() const override {
    return {};
  }
};

} // namespace ros2
} // namespace carla
