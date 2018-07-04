// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ActorBlueprint.h"

#include <boost/algorithm/string/predicate.hpp>

#ifdef _WIN32
#  include <shlwapi.h>
#else
#  include <fnmatch.h>
#endif // _WIN32

namespace carla {
namespace client {

  static bool MatchWildcards(const std::string &str, const std::string &test) {
#ifdef _WIN32
    return PathMatchSpecA(str.c_str(), test.c_str());
#else
    return 0 == fnmatch(test.c_str(), str.c_str(), 0);
#endif // _WIN32
  }

  bool ActorBlueprint::StartsWith(const std::string &test) const {
    return boost::starts_with(GetTypeId(), test);
  }

  bool ActorBlueprint::MatchWildcards(const std::string &test) const {
    return ::carla::client::MatchWildcards(GetTypeId(), test);
  }

} // namespace client
} // namespace carla
