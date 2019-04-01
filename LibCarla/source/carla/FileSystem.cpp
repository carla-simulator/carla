// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/FileSystem.h"

#include "carla/Exception.h"
#include "carla/StringUtil.h"

#include <boost/filesystem/operations.hpp>

namespace carla {

  namespace fs = boost::filesystem;

  void FileSystem::ValidateFilePath(std::string &filepath, const std::string &ext) {
    fs::path path(filepath);
    if (path.extension().empty() && !ext.empty()) {
      if (ext[0] != '.') {
        path += '.';
      }
      path += ext;
    }
    auto parent = path.parent_path();
    if (!parent.empty()) {
      fs::create_directories(parent);
    }
    filepath = path.string();
  }

  std::vector<std::string> FileSystem::ListFolder(
      const std::string &folder_path,
      const std::string &wildcard_pattern) {
    fs::path root(folder_path);
    if (!fs::exists(root) || !fs::is_directory(root)) {
      throw_exception(std::invalid_argument(folder_path + ": no such folder"));
    }

    std::vector<std::string> results;
    fs::directory_iterator end;
    for (fs::directory_iterator it(root); it != end; ++it) {
      if (fs::is_regular_file(*it)) {
        const std::string filename = it->path().filename().string();
        if (StringUtil::Match(filename, wildcard_pattern)) {
          results.emplace_back(filename);
        }
      }
    }
    return results;
  }

} // namespace carla
