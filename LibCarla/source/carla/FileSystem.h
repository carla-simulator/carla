// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>
#include <vector>

namespace carla {

  /// Static functions for accessing the file system.
  ///
  /// @warning Using this file requires linking against boost_filesystem.
  class FileSystem {
  public:

    /// Convenient function to validate a path before creating a file.
    ///
    /// 1)  Ensures all the parent directories are created if missing.
    /// 2)  If @a filepath is missing the extension, @a default_extension is
    ///     appended to the path.
    static void ValidateFilePath(
        std::string &filepath,
        const std::string &default_extension = "");

    /// List (not recursively) regular files at @a folder_path matching
    /// @a wildcard_pattern.
    ///
    /// @throw std::invalid_argument if folder does not exist.
    ///
    /// @todo Do permission check.
    static std::vector<std::string> ListFolder(
        const std::string &folder_path,
        const std::string &wildcard_pattern);
  };

} // namespace carla
