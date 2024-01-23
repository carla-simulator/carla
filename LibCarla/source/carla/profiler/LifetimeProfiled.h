// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace carla {
namespace profiler {

  class LifetimeProfiled {
  protected:

#ifndef LIBCARLA_ENABLE_LIFETIME_PROFILER

    LifetimeProfiled() = default;

#else

    LifetimeProfiled(std::string display_name);

  public:

    ~LifetimeProfiled();

#endif // LIBCARLA_ENABLE_LIFETIME_PROFILER
  };

} // namespace profiler
} // namespace carla

#ifndef LIBCARLA_ENABLE_LIFETIME_PROFILER
#  define LIBCARLA_INITIALIZE_LIFETIME_PROFILER(display_name) ::carla::profiler::LifetimeProfiled()
#else
#  define LIBCARLA_INITIALIZE_LIFETIME_PROFILER(display_name) ::carla::profiler::LifetimeProfiled(display_name)
#endif // LIBCARLA_ENABLE_LIFETIME_PROFILER
