// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

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
  };

} // namespace carla
