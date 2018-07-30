// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <string>

namespace carla {

  class StringUtil {
  public:

    static void ToLower(std::string &str) {
      boost::algorithm::to_lower(str);
    }

    static std::string ToLowerCopy(const std::string &str) {
      return boost::algorithm::to_lower_copy(str);
    }

    template <typename Container>
    static void Split(Container &destination, const std::string &str, const std::string &separators) {
      boost::split(destination, str, boost::is_any_of(separators));
    }

    /// Match @a str with the Unix shell-style @a wildcard_pattern.
    static bool Match(const std::string &str, const std::string &wildcard_pattern);
  };

} // namespace carla
