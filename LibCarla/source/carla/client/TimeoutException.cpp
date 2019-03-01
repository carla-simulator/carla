// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/TimeoutException.h"

namespace carla {
namespace client {

  using namespace std::string_literals;

  TimeoutException::TimeoutException(
      const std::string &endpoint,
      time_duration timeout)
    : std::runtime_error(
        "time-out of "s + std::to_string(timeout.milliseconds()) +
        "ms while waiting for the simulator, "
        "make sure the simulator is ready and connected to " + endpoint) {}

} // namespace client
} // namespace carla
