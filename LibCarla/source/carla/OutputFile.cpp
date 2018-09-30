// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/OutputFile.h"

#include "carla/Logging.h"

#include <boost/filesystem/operations.hpp>

namespace carla {

  static auto ConstructPath(const std::string &path_str, const std::string &ext) {
    namespace fs = boost::filesystem;
    fs::path path(path_str);
    if (path.extension().empty() && !ext.empty()) {
      if (ext[0] != '.') {
        path += '.';
      }
      path += ext;
    }
    fs::create_directories(path.parent_path());
    log_debug("OutputFile: opening", path);
    return path;
  }

  OutputFile::OutputFile(
      const std::string &path,
      const std::string &extension,
      std::ios_base::openmode mode)
    : _ofstream(ConstructPath(path, extension).c_str(), mode) {}

} // namespace carla
