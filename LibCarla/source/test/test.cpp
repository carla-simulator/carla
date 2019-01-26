// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/Exception.h>
#include <carla/Logging.h>

#include <exception>

namespace carla {

  void throw_exception(const std::exception &e) {
    log_error("carla::throw_exception:", e.what());
    throw e;
  }

} // namespace carla
