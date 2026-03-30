// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/dds/IDDSPublisherMiddleware.h"
#include "carla/Logging.h"

namespace carla {
namespace ros2 {

/// CycloneDDS implementation of IDDSPublisherMiddleware (stub).
///
/// This is a placeholder that compiles and satisfies the factory interface
/// but does not contain a real CycloneDDS implementation. All operations
/// log an error and return failure. The real implementation will replace
/// this file once CycloneDDS types and the C API are available.
///
/// Parameterized on a traits type T that provides:
///   T::msg_type  — middleware-neutral POD message struct
template<typename T>
class CycloneDDSPublisherMiddleware : public IDDSPublisherMiddleware {
 public:
  bool Init(const std::string& topic_name) override {
    log_error("CycloneDDSPublisherMiddleware: stub — not yet implemented "
              "(topic '", topic_name, "')");
    return false;
  }

  bool Publish(void* /*message_data*/) override {
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
