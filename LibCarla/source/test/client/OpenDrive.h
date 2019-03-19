// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>
#include <vector>

namespace util {

  /// Helper for loading Test OpenDrive files.
  class OpenDrive {
  public:

    static std::vector<std::string> GetAvailableFiles();

    static std::string Load(const std::string &filename);
  };

} // namespace util
