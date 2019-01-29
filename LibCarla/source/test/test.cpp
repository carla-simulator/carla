// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifdef LIBCARLA_NO_EXCEPTIONS

#include <carla/Exception.h>
#include <carla/Logging.h>

#include <exception>

namespace carla {

  void throw_exception(const std::exception &e) {
    log_critical("carla::throw_exception:", e.what());
    log_critical("calling std::terminate because exceptions are disabled.");
    std::terminate();
  }

} // namespace carla

#endif // LIBCARLA_NO_EXCEPTIONS
