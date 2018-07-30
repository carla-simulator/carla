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

  bool StringUtil::Match(const std::string &str, const std::string &test) {
#ifdef _WIN32
    return PathMatchSpecA(str.c_str(), test.c_str());
#else
    return 0 == fnmatch(test.c_str(), str.c_str(), 0);
#endif // _WIN32
  }

} // namespace carla
