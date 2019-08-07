// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Time.h"

#include <stdexcept>
#include <string>

namespace carla {
namespace client {

  class TimeoutException : public std::runtime_error {
  public:

    explicit TimeoutException(
        const std::string &endpoint,
        time_duration timeout);
  };

} // namespace client
} // namespace carla
