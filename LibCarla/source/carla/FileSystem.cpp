// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/FileSystem.h"

#include <boost/filesystem/operations.hpp>

namespace carla {

  void FileSystem::ValidateFilePath(std::string &filepath, const std::string &ext) {
    namespace fs = boost::filesystem;
    fs::path path(filepath);
    if (path.extension().empty() && !ext.empty()) {
      if (ext[0] != '.') {
        path += '.';
      }
      path += ext;
    }
    fs::create_directories(path.parent_path());
    filepath = path.string();
  }

} // namespace carla
