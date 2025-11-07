// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace ros2 {

class ServiceInterface {
public:
  /**
   * Default constructor.
   */
  ServiceInterface() = default;
  /**
   * Copy operation not allowed due to active subscriptions
   */
  ServiceInterface(const ServiceInterface&) = delete;
  /**
   * Assignment operation not allowed due to active subscriptions
   */
  ServiceInterface& operator=(const ServiceInterface&) = delete;
  /**
   * Move operation not allowed due to active subscriptions
   */
  ServiceInterface(ServiceInterface&&) = delete;
  /**
   * Move operation not allowed due to active subscriptions
   */
  ServiceInterface& operator=(ServiceInterface&&) = delete;

  /**
   * Default destructor.
   */
  virtual ~ServiceInterface() = default;

  /**
   * Check if there is a new request available and execute callback if required
   */
  virtual void CheckRequest() = 0;
};
}  // namespace ros2
}  // namespace carla
