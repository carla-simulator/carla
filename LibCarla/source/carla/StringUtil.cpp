// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/StringUtil.h"

#ifdef _WIN32
#  include <shlwapi.h>
#else
#  include <fnmatch.h>
#endif // _WIN32

namespace carla {

  bool StringUtil::Match(const char *str, const char *test) {
#ifdef _WIN32
    return PathMatchSpecA(str, test);
#else
    return 0 == fnmatch(test, str, 0);
#endif // _WIN32
  }

} // namespace carla
